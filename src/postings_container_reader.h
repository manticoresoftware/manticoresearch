// Immutable postings container reader for on-disk versions 4 through 6.
// It uses mapped catalogs and payloads and does not own a term map.
#pragma once
#include "postings_container_codecs.h"
#include "exact_bm25a_utils.h"
#include "field_norms.h"
#include "std/timers.h"
#include <array>
#include <vector>
namespace e1 {
struct OpenTimings_t { uint64_t header=0,crc=0,structural=0; };
enum class OpenValidationPath_e { NONE, TRUSTED_FAST, DEEP };
// Equality pruning is fail-closed. V9 public-ID minima become visible to
// cursors only after deep open checks this authoritative rowwise accessor,
// or a trusted-fast open carries its separately content-bound proof.
class PublicIDReader_i {
public:
 virtual ~PublicIDReader_i()=default;
 virtual uint32_t Rows()const=0;
 virtual bool Get(uint32_t row,uint64_t&value)const=0;
};
inline uint16_t U16(const uint8_t*p){return p[0]|uint16_t(p[1])<<8;}
// Frequent descriptor: window, ordinal base, card:u16, type:u16, bytes, offset.
struct Block {
 const uint8_t *data=nullptr,*d=nullptr;
 uint32_t id() const{return U32(d);} uint32_t ordinal()const{return U32(d+4);}
 uint32_t card()const{return U16(d+8);} uint32_t type()const{return U16(d+10);}
 uint32_t bytes()const{return U32(d+12);} const uint8_t *payload()const{return data+U64(d+16);}
};
inline void Mask(Block b,std::array<uint64_t,64>&a){
 a.fill(0);auto p=b.payload();
 if(b.type()==1){for(unsigned w=0;w<64;++w)a[w]=U64(p+w*8);return;}
 if(b.type()==0){for(unsigned i=0;i<b.card();++i){auto x=U16(p+i*2);a[x/64]|=uint64_t(1)<<(x%64);}return;}
 for(unsigned i=0;i<b.bytes();i+=4){unsigned lo=U16(p+i),hi=U16(p+i+2),l=lo/64,h=hi/64;auto lm=~uint64_t(0)<<(lo%64),hm=~uint64_t(0)>>(63-hi%64);if(l==h)a[l]|=lm&hm;else{a[l]|=lm;for(unsigned w=l+1;w<h;++w)a[w]=~uint64_t(0);a[h]|=hm;}}
}
struct TermView {
 const uint8_t *data=nullptr,*term=nullptr;
 uint32_t version=0;
 uint32_t DF()const{return term?U32(term):0;} bool Frequent()const{return term&&(U32(term+12)&1)==1;}
 uint32_t FirstFieldTFWidth()const{return term?(U32(term+12)>>8)&63:0;}
 uint32_t Blocks()const{return Frequent()?U32(term+4):0;}
 Block At(uint32_t i)const{return {data,term+24+uint64_t(i)*24};}
};
class Store {
 const uint8_t *m_p=nullptr,*m_dir=nullptr;uint64_t m_n=0;uint32_t m_version=0,m_entrySize=0;bool m_publicIDMinValidated=false;
public:
 const uint8_t *Data()const{return m_p;}
 const uint8_t *Find(uint64_t key)const{
  if(!key||key>m_n)return nullptr;
  if(m_entrySize==16)return m_dir+(key-1)*16;
  uint64_t lo=0,hi=m_n;while(lo<hi){auto mid=lo+(hi-lo)/2;if(U64(m_dir+mid*32)<key)lo=mid+1;else hi=mid;}return lo<m_n&&U64(m_dir+lo*32)==key?m_dir+lo*32:nullptr;
 }
 TermView View(uint64_t key)const{auto d=Find(key);return {m_p,d?m_p+U64(d+(m_entrySize==16?0:8)):nullptr,m_version};}
 bool Stats(uint64_t key,uint32_t&docs,bool&hasHits,uint64_t&hits)const{
  auto d=Find(key);if(!d)return false;
  if(m_entrySize==16){auto h=m_p+U64(d);docs=U32(h);hasHits=(U32(h+12)&2)!=0;hits=U64(d+8);}
  else{docs=U32(d+16);hasHits=U32(d+20)!=0;hits=U64(d+24);}
  return true;
 }
 bool PublicIDMinValidated()const{return m_publicIDMinValidated;}
 bool Open(const uint8_t*p,uint64_t size,uint32_t rows,const uint8_t*dict,uint64_t dictSize,const uint8_t*hits,uint64_t hitSize,std::string&error,bool trusted=false,OpenTimings_t*timings=nullptr,const FieldNormReader_i*pNorms=nullptr,const PublicIDReader_i*pPublicIDs=nullptr,OpenValidationPath_e*pValidationPath=nullptr,bool trustedPublicIDProof=false){
  m_p=nullptr;m_dir=nullptr;m_n=0;m_version=0;m_entrySize=0;m_publicIDMinValidated=false;if(pValidationPath)*pValidationPath=OpenValidationPath_e::NONE;uint64_t stage=timings?MonoMicroTimer():0;auto fail=[&](const char*s){error=std::string("E1: ")+s;return false;};
  bool v4=size>=48&&!memcmp(p,"E1POST04",8)&&U32(p+8)==4;
  bool v5=size>=48&&!memcmp(p,"E1POST05",8)&&U32(p+8)==5;
  bool v6=size>=48&&!memcmp(p,"E1POST06",8)&&U32(p+8)==6;
  bool v7=size>=48&&!memcmp(p,"E1POST07",8)&&U32(p+8)==7;
  bool v9=size>=48&&!memcmp(p,"E1POST09",8)&&U32(p+8)==9;
  if((!v4&&!v5&&!v6&&!v7&&!v9)||U64(p+16)!=size)return fail("format/length");
  uint32_t version=v9?9:v7?7:v6?6:v5?5:4;
  uint32_t header=U32(p+12),flags=U32(p+44);uint64_t nt=U64(p+24),directory=0,end=0,payloadEnd=size;
  if(header==48&&flags==0){m_entrySize=32;directory=48;if(nt>(size-48)/32)return fail("directory overflow");end=48+nt*32;}
  else if(header==56&&flags==1){m_entrySize=16;if(size<56)return fail("streamed header");directory=U64(p+48);if(directory<56||directory>size||nt>(size-directory)/16||directory+nt*16!=size)return fail("streamed directory");end=56;payloadEnd=directory;}
  else return fail("layout/version");
  const bool hasPublicIDBinding=v9&&pPublicIDs;
  const bool validatePublicIDs=hasPublicIDBinding&&pPublicIDs->Rows()==rows;
  const bool useTrustedFast=trusted&&(!v9||(validatePublicIDs&&trustedPublicIDProof));
  if(pValidationPath)*pValidationPath=useTrustedFast?OpenValidationPath_e::TRUSTED_FAST:OpenValidationPath_e::DEEP;
  if(timings){timings->header=MonoMicroTimer()-stage;stage=MonoMicroTimer();}
  if(!useTrustedFast){bool valid=CRC(p+header,size-header)==U32(p+32)&&CRC(dict,dictSize)==U32(p+36)&&CRC(hits,hitSize)==U32(p+40);if(timings){timings->crc=MonoMicroTimer()-stage;stage=MonoMicroTimer();}if(!valid)return fail("checksums");}
  auto dir=p+directory;uint64_t prevKey=0;
  if(useTrustedFast){
   uint64_t prevOff=header;
   for(uint64_t t=0;t<nt;++t){auto d=dir+t*m_entrySize;uint64_t key=m_entrySize==16?t+1:U64(d),off=U64(d+(m_entrySize==16?0:8));auto h=p+off;uint32_t df=off<=payloadEnd&&payloadEnd-off>=24?U32(h):0,has=off<=payloadEnd&&payloadEnd-off>=24?((U32(h+12)>>1)&1):2;if(!key||key<=prevKey||off<prevOff||off>=payloadEnd||payloadEnd-off<24||!df||df>rows||has>1)return fail("trusted term catalog");prevKey=key;prevOff=off;}
   m_p=p;m_dir=dir;m_n=nt;m_version=version;m_publicIDMinValidated=v9&&validatePublicIDs&&trustedPublicIDProof;if(timings)timings->structural=MonoMicroTimer()-stage;return true;
  }
  for(uint64_t t=0;t<nt;++t){
   auto d=dir+t*m_entrySize;auto key=m_entrySize==16?t+1:U64(d),off=U64(d+(m_entrySize==16?0:8));
   if(!key||key<=prevKey||off!=end||off>payloadEnd||payloadEnd-off<24)return fail("term catalog");prevKey=key;
   auto h=p+off;auto df=m_entrySize==16?U32(h):U32(d+16),has=m_entrySize==16?((U32(h+12)>>1)&1):U32(d+20);auto hitsTotal=m_entrySize==16?U64(d+8):U64(d+24);
   if(!df||df>rows||has>1)return fail("term catalog");
   auto nb=U32(h+4),nm=U32(h+8),type=U32(h+12);auto mo=U64(h+16);
   auto frequent=type&1u,fieldWidth=(type>>8)&63u;
   if(U32(h)!=df||frequent!=(df>=4096?1u:0u)||(type&~0x00003f03u)||(version<6&&fieldWidth)||fieldWidth>32||nm!=(uint64_t(df)+127)/128)return fail("term type/DF");
   uint64_t desc=24;
   if(!nb||nb>df||nb>(size-off-24)/desc||(!frequent&&nb!=nm))return fail("row directory bounds");
   end=off+24+uint64_t(nb)*desc;uint32_t ordinal=0,prevRow=0,prevId=0;std::vector<uint32_t> termRows(version>=7?df:0);
   for(uint32_t b=0;b<nb;++b){
    if(frequent){
     Block block{p,h+24+uint64_t(b)*24};auto id=block.id(),card=block.card(),bytes=block.bytes(),codec=block.type();auto po=U64(block.d+16);
     if(block.ordinal()!=ordinal||!card||card>4096||card>df-ordinal||id>=(uint64_t(rows)+4095)/4096||(b&&id<=prevId)||codec>2||po!=end||po>size||bytes>size-po)return fail("container descriptor");
     if((codec==0&&bytes!=card*2)||(codec==1&&bytes!=512)||(codec==2&&(!bytes||bytes%4||bytes>512)))return fail("container length");
     auto q=p+po;uint32_t count=0,previous=0,valid=std::min(4096u,rows-id*4096);
     if(codec==0){for(uint32_t i=0;i<card;++i){auto x=U16(q+i*2);if(x>=valid||(i&&x<=previous))return fail("array order");previous=x;}count=card;}
     else if(codec==1){for(unsigned w=0;w<64;++w){auto v=U64(q+w*8);count+=__builtin_popcountll(v);if(w*64>=valid){if(v)return fail("bitmap row bounds");}else if(valid-w*64<64&&v>>(valid-w*64))return fail("bitmap tail");}}
     else {for(unsigned i=0;i<bytes;i+=4){auto lo=U16(q+i),hi=U16(q+i+2);if(lo>hi||hi>=valid||(i&&lo<=previous+1))return fail("run order");count+=hi-lo+1;previous=hi;}}
     if(count!=card)return fail("container cardinality");if(version>=7){uint32_t at=ordinal;for(uint32_t w=0;w<64;++w){uint64_t bits=0;if(codec==1)bits=U64(q+w*8);else{std::array<uint64_t,64> mask{};Mask(block,mask);bits=mask[w];}while(bits){uint32_t bit=uint32_t(__builtin_ctzll(bits));bits&=bits-1;termRows[at++]=id*4096+w*64+bit;}}if(at!=ordinal+card)return fail("container row materialization");}ordinal+=card;prevId=id;end=po+bytes;
    }else{
     auto g=h+24+uint64_t(b)*24;auto first=U32(g),last=U32(g+4),n=U32(g+8),f=U32(g+12),codec=f&15,width=(f>>16)&63;auto po=U64(g+16);auto span=uint64_t(last)-first+1;
     if(n!=std::min(128u,df-b*128)||first>last||last>=rows||(b&&first<=prevRow)||codec<1||codec>4||(f&~0x003f000fu)||(codec==3?(!width||width>32):width!=0)||(codec==4&&span>1024))return fail("rare descriptor");
     uint64_t bytes=codec==1?0:codec==2?uint64_t(n-1)*4:codec==3?(uint64_t(n-1)*width+7)/8:(span+7)/8;
     if(po!=end||po>size||bytes>size-po)return fail("rare bounds");uint32_t tmp[128];if(!DecodeRows(p+po,first,last,n,f,true,tmp))return fail("rare row codec");if(version>=7)std::copy_n(tmp,n,termRows.data()+ordinal);prevRow=last;ordinal+=n;end=po+bytes;
    }
   }
   if(ordinal!=df||mo!=end||mo>size||uint64_t(nm)*16>size-mo)return fail("ordinal/metadata directory");
   end=mo+uint64_t(nm)*16;uint64_t sum=0;std::vector<uint8_t> expected((uint64_t(df)+63)/64,0);std::vector<uint32_t> termTF(version>=7?df:0);
   for(uint32_t b=0;b<nm;++b){auto g=p+mo+uint64_t(b)*16;auto po=U64(g);auto f=U32(g+8),n=U32(g+12);
    auto tm=(f>>4)&3,mm=(f>>6)&3,tw=(f>>9)&63,mw=(f>>23)&63;bool split=f&(1u<<29);
    if(n!=std::min(128u,df-b*128)||(f&~0x3fff7ef0u)||tm==3||(((f>>22)&1)!=has)||(tm==2?(!tw||tw>32):tw)||(mm==2?(mw>32):mw)||(split&&!has))return fail("metadata flags");
    auto bytes=Meta4Bytes(f,n);if(po!=end||po>size||bytes>size-po)return fail("metadata bounds");
    for(uint32_t i=0;i<n;++i){uint32_t tf,mask;uint64_t ref;if(!Metadata4(p+po,f,n,i,tf,mask,ref))return fail("reference overflow");if(!tf)return fail("zero TF");sum+=tf;auto oi=uint64_t(b)*128+i,bi=oi/64;if(version>=7)termTF[oi]=tf;else expected[bi]=std::max(expected[bi],uint8_t(std::min(tf,255u)));
     if(has){if(ref>>63){if(tf!=1||(ref&0x7fffffff00000000ULL)||!uint32_t(ref))return fail("inline hit");}
      else{if(!ref||ref>=hitSize)return fail("hit reference");uint64_t o=ref,v=0,raw=0;for(uint32_t j=0;j<tf;++j){if(!Var(hits,hitSize,o,v)||!v||v>UINT32_MAX-raw)return fail("position delta");raw+=v;}if(!Var(hits,hitSize,o,v)||v)return fail("hit length");}}
     else if(ref>>63)return fail("hitless inline");
    }end=po+bytes;
   }if(sum!=hitsTotal)return fail("hit sum");
   if(version>=5){if(expected.size()>payloadEnd-end)return fail("bounds tail");if(version<7){for(size_t i=0;i<expected.size();++i)if(p[end+i]!=expected[i])return fail("unsafe maxTF bound");}else for(size_t block=0;block<expected.size();++block){const uint8_t stored=p[end+block];if(stored==255)continue;if(!pNorms||pNorms->Rows()!=rows)return fail("finite BM25A bound without authoritative norms");const uint32_t first=uint32_t(block*64),count=std::min(64u,df-first);uint32_t dl[64];if(!pNorms->GatherTotal(termRows.data()+first,count,dl))return fail("BM25A norm lookup");uint8_t need=0;for(uint32_t i=0;i<count;++i)need=std::max(need,E1EncodeBM25A12_075_256(termTF[first+i],dl[i]));if(stored<need)return fail("unsafe BM25A ratio bound");}end+=expected.size();}
   if(version==9){const uint64_t minIdBytes=uint64_t(expected.size())*8;if(minIdBytes>payloadEnd-end)return fail("public ID bounds tail");if(validatePublicIDs){for(size_t block=0;block<expected.size();++block){const uint32_t first=uint32_t(block*64),count=std::min(64u,df-first);uint64_t actual=UINT64_MAX;for(uint32_t i=0;i<count;++i){uint64_t id=0;if(!pPublicIDs->Get(termRows[first+i],id))return fail("authoritative public ID lookup");actual=std::min(actual,id);}if(U64(p+end+block*8)>actual)return fail("unsafe public ID minimum");}}end+=minIdBytes;}
   if(version>=6&&fieldWidth){
    const uint64_t fieldBytes=(uint64_t(df)*fieldWidth+7)/8;if(fieldBytes>payloadEnd-end)return fail("field TF bounds");
    const uint8_t* fieldTF=p+end;
    for(uint32_t o=0;o<df;++o){
     const uint32_t group=o/128,slot=o%128;auto g=p+mo+uint64_t(group)*16;uint32_t tf,mask;uint64_t ref;
     if(!Metadata4(p+U64(g),U32(g+8),U32(g+12),slot,tf,mask,ref))return fail("field TF metadata");
     const uint32_t first=uint32_t(PackedBounded(fieldTF,fieldWidth,o,df));
     if(first&&(__builtin_popcount(mask)!=2||first>=tf))return fail("field TF value");
    }
    end+=fieldBytes;
   }
  }
  if(end!=payloadEnd)return fail("trailing bytes");m_p=p;m_dir=dir;m_n=nt;m_version=version;m_publicIDMinValidated=validatePublicIDs;if(timings)timings->structural=MonoMicroTimer()-stage;return true;
 }
};
class Cursor {
 const uint8_t*m_p=nullptr,*m_term=nullptr;uint32_t m_df=0,m_next=0,m_cached=UINT32_MAX,m_rows[128];
 const uint8_t*m_bounds=nullptr,*m_minPublicIDs=nullptr,*m_firstFieldTF=nullptr;uint32_t m_firstFieldTFWidth=0,m_version=0;
 uint32_t m_block=0,m_lastOrdinal=UINT32_MAX,m_lastRow=0,m_bitmapBlock=UINT32_MAX,m_bitmapWord=0,m_bitmapEndOrdinal=0,m_bitmapWindow=0;uint64_t m_bitmapRemaining=0;const uint8_t*m_bitmapPayload=nullptr;
 uint32_t m_meta=UINT32_MAX,m_tf[128],m_mask[128];uint64_t m_ref[128];
 uint64_t m_uMetaDecoded=0;
 uint32_t m_dBatchStage[4] { UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX },m_dBatchTF[128];
 uint32_t m_uDirectWindow=UINT32_MAX,m_uDirectBlock=UINT32_MAX;
 uint32_t m_uHintPrefixBlock=UINT32_MAX;
 std::array<uint16_t,65> m_dHintPrefix{};
 std::array<uint64_t,64> m_dDirectMask{};
 std::array<uint16_t,65> m_dDirectPrefix{};
 const uint64_t *m_pSelectedMeta=nullptr;uint32_t m_uSelectedWord=0,m_uSelectedOrdinal=0;uint64_t m_uSelectedMembers=0;
 std::vector<uint32_t> m_dRankOrder;
 uint32_t m_uRankPos=0,m_uRankInBlock=0;
 bool m_bRankOrderReady=false;
 void PrepareRankedOrder(uint64_t&entries,uint64_t&buckets){
  if(m_bRankOrderReady)return;m_bRankOrderReady=true;if(BoundKind()==E1RankedBoundKind_e::NONE)return;
  const uint32_t n=(m_df+63)/64;std::array<uint32_t,256> counts{},offsets{},next{};
  for(uint32_t i=0;i<n;++i)++counts[m_bounds[i]];entries+=n;
  uint32_t at=0;for ( int iBucket=255; iBucket>=0; --iBucket ){if(counts[iBucket])++buckets;offsets[iBucket]=at;at+=counts[iBucket];}
  next=offsets;m_dRankOrder.resize(n);
  for(uint32_t i=0;i<n;++i)m_dRankOrder[next[m_bounds[i]]++]=i;
 }
public:
 bool Active()const{return m_p;} void Reset(){m_next=0;m_cached=UINT32_MAX;m_meta=UINT32_MAX;for(auto&m:m_dBatchStage)m=UINT32_MAX;m_block=0;m_lastOrdinal=UINT32_MAX;m_bitmapBlock=UINT32_MAX;m_bitmapWord=0;m_bitmapEndOrdinal=0;m_bitmapWindow=0;m_bitmapRemaining=0;m_bitmapPayload=nullptr;m_dRankOrder.clear();m_uRankPos=0;m_uRankInBlock=0;m_bRankOrderReady=false;m_uMetaDecoded=0;m_uDirectWindow=UINT32_MAX;m_uDirectBlock=UINT32_MAX;m_uHintPrefixBlock=UINT32_MAX;m_dHintPrefix.fill(0);m_dDirectMask.fill(0);m_dDirectPrefix.fill(0);m_pSelectedMeta=nullptr;m_uSelectedWord=0;m_uSelectedOrdinal=0;m_uSelectedMembers=0;}
 uint64_t TakeMetadataGroupsDecoded(){auto u=m_uMetaDecoded;m_uMetaDecoded=0;return u;}
 void Bind(const Store&s,uint64_t key){auto v=s.View(key);m_p=v.term?v.data:nullptr;m_term=v.term;m_df=v.DF();m_version=v.version;m_bounds=nullptr;m_minPublicIDs=nullptr;m_firstFieldTF=nullptr;m_firstFieldTFWidth=0;if(m_term&&m_version>=5){auto nm=U32(m_term+8);auto mo=U64(m_term+16);if(nm){auto g=m_p+mo+uint64_t(nm-1)*16;m_bounds=m_p+U64(g)+Meta4Bytes(U32(g+8),U32(g+12));const uint64_t blocks=(uint64_t(m_df)+63)/64;if(m_version==9&&s.PublicIDMinValidated())m_minPublicIDs=m_bounds+blocks;if(m_version>=6&&(m_firstFieldTFWidth=v.FirstFieldTFWidth()))m_firstFieldTF=m_bounds+blocks+(m_version==9?blocks*8:0);}}Reset();}
 TermView View()const{return {m_p,m_term,m_version};}
 E1RankedBoundKind_e BoundKind()const{return !m_bounds?E1RankedBoundKind_e::NONE:m_version>=5&&m_version<=6?E1RankedBoundKind_e::MAX_TF:(m_version==7||m_version==9)?E1RankedBoundKind_e::BM25A_RATIO:E1RankedBoundKind_e::NONE;}
 bool HasRankedBounds()const{return BoundKind()==E1RankedBoundKind_e::MAX_TF;}
 bool HasBM25ARatioBounds()const{return m_bounds&&(m_version==7||m_version==9);}
 bool HasPublicIdMinBounds()const{return m_minPublicIDs&&m_version==9;}
 bool PublicIdMinBound(uint32_t block,uint64_t&value)const{if(!HasPublicIdMinBounds()||block>=(m_df+63)/64)return false;value=U64(m_minPublicIDs+uint64_t(block)*8);return true;}
 bool BM25ARatioBound(uint32_t block,uint8_t&code)const{if(!HasBM25ARatioBounds()||block>=(m_df+63)/64)return false;code=m_bounds[block];return true;}
 bool ExactFieldTF(uint32_t ordinal,uint32_t field,uint32_t mask,uint32_t aggregate,uint32_t&tf)const{
  if(ordinal>=m_df||field>=32||!(mask&(uint32_t(1)<<field)))return false;
  const uint32_t fields=uint32_t(__builtin_popcount(mask));
  if(fields==1){tf=aggregate;return true;}
  if(fields!=2||!m_firstFieldTF||!m_firstFieldTFWidth)return false;
  const uint32_t first=uint32_t(PackedBounded(m_firstFieldTF,m_firstFieldTFWidth,ordinal,m_df));
  if(!first||first>=aggregate)return false;
  tf=field==uint32_t(__builtin_ctz(mask))?first:aggregate-first;return tf!=0;
 }
 bool DirectContainerSupported()const{return BoundKind()!=E1RankedBoundKind_e::NONE&&View().Frequent();}
 bool DirectOrSupported()const{return HasRankedBounds()&&View().Frequent();}
 bool DirectLastWindow(uint32_t&window)const{auto v=View();if(!DirectContainerSupported()||!v.Blocks())return false;window=v.At(v.Blocks()-1).id();return true;}
 bool DirectWindow(uint32_t window,uint64_t*outMask,uint8_t*outBounds,uint32_t&card,uint32_t&maxTF,uint64_t&boundReads){
  card=0;maxTF=0;if(!DirectContainerSupported()||(outBounds&&!HasRankedBounds()))return false;auto v=View();uint32_t lo=0,hi=v.Blocks();while(lo<hi){auto mid=lo+(hi-lo)/2;if(v.At(mid).id()<window)lo=mid+1;else hi=mid;}if(lo==v.Blocks()||v.At(lo).id()!=window)return false;
  auto b=v.At(lo);Mask(b,m_dDirectMask);if(outMask)for(unsigned w=0;w<64;++w)outMask[w]=m_dDirectMask[w];card=b.card();
  const uint32_t first=b.ordinal()/64,last=(b.ordinal()+b.card()-1)/64;boundReads+=last-first+1;
  if(HasRankedBounds())for(uint32_t i=first;i<=last;++i){const uint32_t x=m_bounds[i];if(x==255)maxTF=UINT32_MAX;else if(maxTF!=UINT32_MAX)maxTF=std::max(maxTF,x);}else maxTF=UINT32_MAX;
  if(outBounds){uint32_t ordinal=b.ordinal();for(unsigned w=0;w<64;++w){uint64_t bits=m_dDirectMask[w];while(bits){auto bit=uint32_t(__builtin_ctzll(bits));bits&=bits-1;outBounds[w*64+bit]=m_bounds[ordinal++/64];}}}
  m_dDirectPrefix[0]=0;for(unsigned w=0;w<64;++w)m_dDirectPrefix[w+1]=uint16_t(m_dDirectPrefix[w]+__builtin_popcountll(m_dDirectMask[w]));m_uDirectWindow=window;m_uDirectBlock=lo;return true;
 }
 bool DirectWindowClasses(uint32_t window,uint64_t*outMask,E1DirectBoundWord_t*outWords,uint32_t&card,uint64_t&boundReads){
  card=0;if(!DirectOrSupported()||!outMask||!outWords)return false;auto v=View();uint32_t lo=0,hi=v.Blocks();while(lo<hi){auto mid=lo+(hi-lo)/2;if(v.At(mid).id()<window)lo=mid+1;else hi=mid;}if(lo==v.Blocks()||v.At(lo).id()!=window)return false;
  auto b=v.At(lo);Mask(b,m_dDirectMask);card=b.card();const uint32_t first=b.ordinal()/64,last=(b.ordinal()+b.card()-1)/64;boundReads+=last-first+1;
  uint32_t ordinal=b.ordinal();m_dDirectPrefix[0]=0;for(unsigned w=0;w<64;++w){const uint64_t members=m_dDirectMask[w];outMask[w]=members;E1BuildDirectBoundWord(members,ordinal,m_bounds+ordinal/64,outWords[w]);const uint32_t count=uint32_t(__builtin_popcountll(members));ordinal+=count;m_dDirectPrefix[w+1]=uint16_t(m_dDirectPrefix[w]+count);}
  m_uDirectWindow=window;m_uDirectBlock=lo;return true;
 }
 bool DirectBoundWord(uint32_t window,uint32_t word,uint64_t*outMasks,uint8_t*outBounds,uint32_t&classes){
  classes=0;if(!outMasks||!outBounds||word>=64||m_uDirectWindow!=window)return false;
  const uint64_t members=m_dDirectMask[word];if(!members)return true;
  const auto b=View().At(m_uDirectBlock);const uint32_t ordinal=b.ordinal()+m_dDirectPrefix[word],count=uint32_t(__builtin_popcountll(members));
  const uint32_t first=std::min(count,64u-(ordinal&63));outBounds[0]=m_bounds[ordinal/64];
  if(first==count){outMasks[0]=members;classes=1;return true;}
  uint32_t lo=0,hi=64;
  while(lo<hi){const uint32_t mid=(lo+hi)/2;const uint64_t prefix=mid==64?~uint64_t(0):((uint64_t(1)<<mid)-1);if(uint32_t(__builtin_popcountll(members&prefix))<first)lo=mid+1;else hi=mid;}
  const uint64_t prefix=lo==64?~uint64_t(0):((uint64_t(1)<<lo)-1);outMasks[0]=members&prefix;outMasks[1]=members&~prefix;outBounds[1]=m_bounds[ordinal/64+1];classes=2;return true;
 }
 bool ProbeTF(uint32_t row,uint32_t&tf){
  if(!DirectContainerSupported())return false;const uint32_t window=row/4096,local=row%4096;if(m_uDirectWindow!=window){uint32_t card=0,maxTF=0;uint64_t reads=0;if(!DirectWindow(window,m_dDirectMask.data(),nullptr,card,maxTF,reads))return false;}
  const uint32_t word=local/64,bit=local%64;if(!(m_dDirectMask[word]&(uint64_t(1)<<bit)))return false;
  auto b=View().At(m_uDirectBlock);const uint32_t ordinal=b.ordinal()+m_dDirectPrefix[word]+__builtin_popcountll(m_dDirectMask[word]&((bit==0)?0:(uint64_t(1)<<bit)-1));const uint32_t group=ordinal/128;
  if(m_meta!=group){auto g=m_p+U64(m_term+16)+uint64_t(group)*16;auto n=U32(g+12);Metadata4Block(m_p+U64(g),U32(g+8),n,m_tf,m_mask,m_ref);m_meta=group;++m_uMetaDecoded;}tf=m_tf[ordinal%128];return true;
 }
 bool ExtractWindowTFBatch(uint32_t window,const uint64_t*selected,uint32_t*outTF,uint64_t&requested,uint64_t&written,uint64_t&decoded,uint32_t stage=0){
  if(!DirectContainerSupported()||!selected||!outTF||stage>=4)return false;
  for(unsigned w=0;w<64;++w)requested+=__builtin_popcountll(selected[w]);
  if(m_uDirectWindow!=window){uint32_t card=0,maxTF=0;uint64_t reads=0;if(!DirectWindow(window,m_dDirectMask.data(),nullptr,card,maxTF,reads))return false;}
  auto b=View().At(m_uDirectBlock);uint32_t ordinal=b.ordinal();
  for(unsigned w=0;w<64;++w){uint64_t members=m_dDirectMask[w];while(members){const uint32_t bit=uint32_t(__builtin_ctzll(members));members&=members-1;const uint32_t local=w*64+bit;if(selected[w]&(uint64_t(1)<<bit)){
    const uint32_t group=ordinal/128;if(m_dBatchStage[stage]!=group){auto g=m_p+U64(m_term+16)+uint64_t(group)*16;auto n=U32(g+12);uint32_t masks[128];uint64_t refs[128];Metadata4Block(m_p+U64(g),U32(g+8),n,m_dBatchTF,masks,refs);m_dBatchStage[stage]=group;++decoded;}
    outTF[local]=m_dBatchTF[ordinal%128];++written;
   }++ordinal;}}
  return true;
 }
 bool ExtractWindowMetaBatch(uint32_t window,const uint64_t*selected,uint32_t*outTF,uint32_t*outMask,uint64_t*outRef,uint64_t&requested,uint64_t&written,uint64_t&decoded){
   if(!DirectContainerSupported()||!selected||!outTF||!outMask||!outRef)return false;
   for(unsigned w=0;w<64;++w)requested+=__builtin_popcountll(selected[w]);
   if(m_uDirectWindow!=window){uint32_t card=0,maxTF=0;uint64_t reads=0;if(!DirectWindow(window,m_dDirectMask.data(),nullptr,card,maxTF,reads))return false;}
   auto b=View().At(m_uDirectBlock);uint32_t ordinal=b.ordinal();uint32_t cached=UINT32_MAX,tf[128],mask[128];uint64_t ref[128];
   for(unsigned w=0;w<64;++w){uint64_t members=m_dDirectMask[w];while(members){const uint32_t bit=uint32_t(__builtin_ctzll(members));members&=members-1;const uint32_t local=w*64+bit;if(selected[w]&(uint64_t(1)<<bit)){
     const uint32_t group=ordinal/128;if(cached!=group){auto g=m_p+U64(m_term+16)+uint64_t(group)*16;auto n=U32(g+12);Metadata4Block(m_p+U64(g),U32(g+8),n,tf,mask,ref);cached=group;++decoded;}
     outTF[local]=tf[ordinal%128];outMask[local]=mask[ordinal%128];outRef[local]=ref[ordinal%128];++written;
    }++ordinal;}}
   return true;
  }
 bool BeginSelectedMeta(uint32_t window,const uint64_t*selected,uint64_t&requested){
  if(!DirectContainerSupported()||!selected)return false;
  if(m_uDirectWindow!=window){uint32_t card=0,maxTF=0;uint64_t reads=0;if(!DirectWindow(window,m_dDirectMask.data(),nullptr,card,maxTF,reads))return false;}
  requested=0;for(unsigned w=0;w<64;++w)requested+=__builtin_popcountll(selected[w]);
  m_pSelectedMeta=selected;m_uSelectedWord=0;m_uSelectedOrdinal=View().At(m_uDirectBlock).ordinal();m_uSelectedMembers=m_dDirectMask[0];return true;
 }
 bool NextSelectedMeta(E1SelectedMeta_t&out,uint64_t&decoded,uint32_t scopedField=UINT32_MAX){
  if(!m_pSelectedMeta)return false;
  while(m_uSelectedWord<64){
   while(m_uSelectedMembers){const uint32_t bit=uint32_t(__builtin_ctzll(m_uSelectedMembers));m_uSelectedMembers&=m_uSelectedMembers-1;const uint32_t ordinal=m_uSelectedOrdinal++;if(!(m_pSelectedMeta[m_uSelectedWord]&(uint64_t(1)<<bit)))continue;
    const uint32_t group=ordinal/128;if(m_meta!=group){auto g=m_p+U64(m_term+16)+uint64_t(group)*16;auto n=U32(g+12);Metadata4Block(m_p+U64(g),U32(g+8),n,m_tf,m_mask,m_ref);m_meta=group;++decoded;}
    const uint32_t slot=ordinal%128;out={m_uSelectedWord*64+bit,ordinal,m_tf[slot],m_mask[slot],0,m_ref[slot]};
    if(scopedField<32&&(out.m_uMask&(uint32_t(1)<<scopedField))){
     const uint32_t fields=uint32_t(__builtin_popcount(out.m_uMask));
     if(fields==1)out.m_uScopedTF=out.m_uTF;
     else if(fields==2&&m_firstFieldTF&&m_firstFieldTFWidth){const uint32_t first=uint32_t(PackedBounded(m_firstFieldTF,m_firstFieldTFWidth,ordinal,m_df));if(first&&first<out.m_uTF)out.m_uScopedTF=scopedField==uint32_t(__builtin_ctz(out.m_uMask))?first:out.m_uTF-first;}
    }
    return true;}
   if(++m_uSelectedWord<64)m_uSelectedMembers=m_dDirectMask[m_uSelectedWord];
  }
  m_pSelectedMeta=nullptr;return false;
 }
 uint32_t Row(uint32_t ordinal){
  if(!View().Frequent()){auto b=ordinal/128;auto g=m_term+24+uint64_t(b)*24;if(m_cached!=b){DecodeRows(m_p+U64(g+16),U32(g),U32(g+4),U32(g+8),U32(g+12),true,m_rows);m_cached=b;}return m_rows[ordinal%128];}
  if(m_lastOrdinal==ordinal)return m_lastRow;
  if(m_lastOrdinal!=UINT32_MAX&&ordinal==m_lastOrdinal+1&&ordinal<m_bitmapEndOrdinal&&m_bitmapBlock==m_block){uint32_t w=m_bitmapWord;uint64_t bits=m_bitmapRemaining;while(!bits&&++w<64)bits=U64(m_bitmapPayload+w*8);const uint32_t x=w*64+__builtin_ctzll(bits);m_bitmapWord=w;m_bitmapRemaining=bits&(bits-1);m_lastOrdinal=ordinal;m_lastRow=m_bitmapWindow*4096+x;return m_lastRow;}
  auto v=View();auto b=v.At(m_block);
  if(ordinal==b.ordinal()+b.card()&&m_block+1<v.Blocks()){b=v.At(++m_block);}
  if(ordinal<b.ordinal()||ordinal>=b.ordinal()+b.card()){uint32_t lo=0,hi=v.Blocks();while(lo<hi){auto mid=lo+(hi-lo)/2;auto x=v.At(mid);if(x.ordinal()+x.card()<=ordinal)lo=mid+1;else hi=mid;}m_block=lo;b=v.At(lo);}
  auto q=b.payload();uint32_t local=ordinal-b.ordinal(),x=0;
  if(b.type()==0)x=U16(q+local*2);
  else if(b.type()==1){uint32_t w=0;uint64_t bits=0;
   for(;w<64;++w){bits=U64(q+w*8);auto n=uint32_t(__builtin_popcountll(bits));if(local<n)break;local-=n;}while(local--)bits&=bits-1;x=w*64+__builtin_ctzll(bits);
   m_bitmapBlock=m_block;m_bitmapWord=w;m_bitmapEndOrdinal=b.ordinal()+b.card();m_bitmapWindow=b.id();m_bitmapRemaining=bits&(bits-1);m_bitmapPayload=q;
  }else{for(uint32_t i=0;i<b.bytes();i+=4){auto lo=U16(q+i),hi=U16(q+i+2);auto n=uint32_t(hi-lo+1);if(local<n){x=lo+local;break;}local-=n;}}
  m_lastOrdinal=ordinal;m_lastRow=b.id()*4096+x;return m_lastRow;
 }
 bool Next(uint32_t&row,uint32_t*tf=nullptr,uint32_t*mask=nullptr,uint64_t*ref=nullptr){if(m_next>=m_df){row=UINT32_MAX;return false;}auto o=m_next++;row=Row(o);if(tf){auto group=o/128;if(m_meta!=group){auto g=m_p+U64(m_term+16)+uint64_t(group)*16;auto n=U32(g+12);Metadata4Block(m_p+U64(g),U32(g+8),n,m_tf,m_mask,m_ref);m_meta=group;++m_uMetaDecoded;}*tf=m_tf[o%128];*mask=m_mask[o%128];*ref=m_ref[o%128];}return true;}
 bool NextRanked(uint32_t&row,uint32_t&tf,uint32_t&mask,uint64_t&ref,uint32_t uMinTF,uint64_t&entries,uint64_t&buckets,uint64_t&selected,uint64_t&skipped,uint64_t&skippedDocs,uint64_t&decoded,const uint64_t*pEligibility=nullptr,uint32_t uEligibilityWords=0,uint64_t*pIneligibleBeforeTF=nullptr,uint32_t uEqualBoundTF=0,uint64_t uWorstTieKey=UINT64_MAX,uint32_t uKnownMask=0,float fRatioIDF=0.0f,int iThreshold=0,uint64_t*equalitySkipped=nullptr){
  PrepareRankedOrder(entries,buckets);
  while(m_uRankPos<m_dRankOrder.size()){
   const uint32_t block=m_dRankOrder[m_uRankPos],begin=block*64,n=std::min(64u,m_df-begin);
   if(!m_uRankInBlock){
    const uint8_t uCode=m_bounds[block];const uint32_t uUpper=uCode==255?UINT32_MAX:uCode;
    const bool bRatio=BoundKind()==E1RankedBoundKind_e::BM25A_RATIO;
    const int upperWeight=bRatio?E1SafeUpperRatioWeight(uCode,fRatioIDF):0;
    const uint64_t minPublicID=m_minPublicIDs?U64(m_minPublicIDs+uint64_t(block)*8):UINT64_MAX;
    const bool ratioTieReject=bRatio&&uCode!=255&&iThreshold>0&&upperWeight==iThreshold&&uWorstTieKey!=UINT64_MAX&&minPublicID!=UINT64_MAX&&minPublicID>=uWorstTieKey;
    const bool tieReject=!bRatio&&uEqualBoundTF&&uWorstTieKey<=UINT32_MAX&&(uEqualBoundTF==UINT32_MAX||uUpper<=uEqualBoundTF)&&Row(begin)>=uWorstTieKey;
    if(ratioTieReject){
     if(equalitySkipped)++*equalitySkipped;++skipped;skippedDocs+=n;++m_uRankPos;m_uRankInBlock=0;continue;
    }
    if((bRatio?E1RatioBoundReject(uCode,fRatioIDF,iThreshold):(uMinTF&&uUpper<uMinTF))||tieReject){
     skipped+=m_dRankOrder.size()-m_uRankPos;
     for(uint32_t i=m_uRankPos;i<m_dRankOrder.size();++i)skippedDocs+=std::min(64u,m_df-m_dRankOrder[i]*64);
     m_uRankPos=m_dRankOrder.size();row=UINT32_MAX;return false;
    }
    ++selected;
   }
   const uint32_t o=begin+m_uRankInBlock++;
   if(m_uRankInBlock==n){++m_uRankPos;m_uRankInBlock=0;}
   row=Row(o);
   if(pEligibility&&(row/64>=uEligibilityWords||!(pEligibility[row/64]&(uint64_t(1)<<(row%64))))){if(pIneligibleBeforeTF)++*pIneligibleBeforeTF;continue;}
   if(uKnownMask){tf=0;mask=uKnownMask;ref=0;return true;}
   const auto group=o/128;
   if(m_meta!=group){auto g=m_p+U64(m_term+16)+uint64_t(group)*16;auto count=U32(g+12);Metadata4Block(m_p+U64(g),U32(g+8),count,m_tf,m_mask,m_ref);m_meta=group;++decoded;}
   tf=m_tf[o%128];mask=m_mask[o%128];ref=m_ref[o%128];return true;
  }row=UINT32_MAX;return false;
 }
 uint32_t NextRows(uint32_t*out,uint32_t cap,uint32_t&last){uint32_t n=0;while(n<cap&&m_next<m_df)out[n++]=Row(m_next++);if(n<cap)last=UINT32_MAX;else if(n)last=out[n-1];return n;}
 bool OrCountWindow(uint32_t,uint64_t*,uint32_t,uint32_t&){return false;}
 void Hint(uint32_t target){
  if(m_next>=m_df||Row(m_next)>=target)return;
  auto v=View();
  if(!v.Frequent()){uint32_t lo=m_next/128,hi=U32(m_term+4);while(lo<hi){auto mid=lo+(hi-lo)/2;if(U32(m_term+24+uint64_t(mid)*24+4)<target)lo=mid+1;else hi=mid;}m_next=std::max(m_next,std::min(m_df,lo*128));while(m_next<m_df&&Row(m_next)<target)++m_next;return;}
  const uint32_t targetBlock=target/4096;uint32_t lo=m_block,hi=v.Blocks();
  if(lo<hi&&v.At(lo).id()<targetBlock)++lo;
  if(lo<hi&&v.At(lo).id()<targetBlock)while(lo<hi){auto mid=lo+(hi-lo)/2;if(v.At(mid).id()<targetBlock)lo=mid+1;else hi=mid;}
  if(lo==v.Blocks()){m_next=m_df;return;}m_block=lo;auto b=v.At(lo);uint32_t rank=0,x=b.id()==target/4096?target%4096:0,hintWord=0,hintRow=UINT32_MAX;uint64_t hintBits=0;auto q=b.payload();
  if(b.id()==target/4096){if(b.type()==0){uint32_t l=0,h=b.card();while(l<h){auto m=l+(h-l)/2;if(U16(q+m*2)<x)l=m+1;else h=m;}rank=l;}
   else if(b.type()==1){if(m_uHintPrefixBlock!=lo){m_dHintPrefix[0]=0;for(uint32_t w=0;w<64;++w)m_dHintPrefix[w+1]=uint16_t(m_dHintPrefix[w]+__builtin_popcountll(U64(q+w*8)));m_uHintPrefixBlock=lo;}rank=m_dHintPrefix[x/64];if(x%64)rank+=__builtin_popcountll(U64(q+(x/64)*8)&((uint64_t(1)<<(x%64))-1));}
   else {for(uint32_t i=0;i<b.bytes();i+=4){auto a=U16(q+i),z=U16(q+i+2);if(a>=x)break;rank+=std::min(uint32_t(z)+1,x)-a;}}
  }
  if(b.type()==1&&rank<b.card()){hintWord=x/64;hintBits=U64(q+hintWord*8)&(~uint64_t(0)<<(x%64));while(!hintBits&&++hintWord<64)hintBits=U64(q+hintWord*8);if(hintWord<64)hintRow=b.id()*4096+hintWord*64+__builtin_ctzll(hintBits);}
  const uint32_t hintedOrdinal=b.ordinal()+rank;m_next=std::max(m_next,hintedOrdinal);
  if(hintRow!=UINT32_MAX&&m_next==hintedOrdinal){m_lastOrdinal=m_next;m_lastRow=hintRow;m_bitmapBlock=m_block;m_bitmapWord=hintWord;m_bitmapEndOrdinal=b.ordinal()+b.card();m_bitmapWindow=b.id();m_bitmapRemaining=hintBits&(hintBits-1);m_bitmapPayload=q;}else m_lastOrdinal=UINT32_MAX;
 }
};
// Ports the validated sidecar Count/CountHybrid window/checkpoint policy, but
// reads compact primary descriptors in place. Rare streams are primary cursors.
template<class Next,class Check> uint64_t Count(const std::vector<TermView>&terms,bool isAnd,size_t streams,Next next,Check checkpoint){
 std::array<uint32_t,32> pos{},pending{};uint64_t count=0;if(checkpoint())return count;for(size_t i=0;i<streams;++i)pending[i]=next(i);
 for(;;){uint32_t id=UINT32_MAX;for(size_t i=0;i<terms.size();++i)if(pos[i]<terms[i].Blocks())id=std::min(id,terms[i].At(pos[i]).id());for(size_t i=0;i<streams;++i)if(pending[i]!=UINT32_MAX)id=std::min(id,pending[i]/4096);if(checkpoint()||id==UINT32_MAX)return count;
  std::array<uint64_t,64>a,b;a.fill(isAnd?~uint64_t(0):0);bool missing=false;
  for(size_t i=0;i<terms.size();++i){if(checkpoint())return count;if(pos[i]>=terms[i].Blocks()||terms[i].At(pos[i]).id()!=id){if(isAnd)missing=true;continue;}Mask(terms[i].At(pos[i]++),b);for(unsigned w=0;w<64;++w)a[w]=isAnd?(a[w]&b[w]):(a[w]|b[w]);}
  for(size_t i=0;i<streams;++i){if(checkpoint())return count;unsigned budget=0;while(pending[i]!=UINT32_MAX&&pending[i]/4096==id){auto x=pending[i]%4096;a[x/64]|=uint64_t(1)<<(x%64);pending[i]=next(i);if(++budget==128){budget=0;if(checkpoint())return count;}}}
  if(checkpoint())return count;if(!missing)for(auto w:a)count+=__builtin_popcountll(w);if(checkpoint())return count;
 }
}
}
