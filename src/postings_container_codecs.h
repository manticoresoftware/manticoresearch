// Experimental immutable primary postings. Explicit LE, bounded mmap cursor.
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <algorithm>
#include <array>
#include <mutex>
#include <unordered_map>
#include "digest_sha1.h"
#if !defined(_WIN32)
#include <sys/stat.h>
#endif
namespace e1 {
constexpr uint32_t VERSION = 0x45310009;
constexpr uint32_t VERSION7 = 0x45310007;
constexpr uint32_t VERSION6 = 0x45310006;
constexpr uint32_t VERSION5 = 0x45310005;
constexpr uint32_t VERSION4 = 0x45310004;
inline std::mutex & TrustedGenerationsMutex() { static std::mutex tMutex; return tMutex; }
using PublicIDDigest_t = HASH20_t;
struct TrustedGeneration_t { uint32_t m_uPublicIDRows = 0; PublicIDDigest_t m_dPublicIDs {}; bool m_bHasPublicIDs = false; };
struct TrustedGenerationProof_t { bool m_bGeneration = false; bool m_bPublicIDs = false; };
inline std::unordered_multimap<std::string,TrustedGeneration_t> & TrustedGenerations() { static std::unordered_multimap<std::string,TrustedGeneration_t> dKeys; return dKeys; }
template<typename GET>
inline bool PublicIDContentDigest ( uint32_t uRows, GET fnGet, PublicIDDigest_t & dDigest )
{
 SHA1_c tHash;
 tHash.Init();
 static const uint8_t dDomain[] = { 'E','1','P','U','B','L','I','C','I','D',1 };
 tHash.Update ( dDomain, int(sizeof(dDomain)) );
 uint8_t dValues[4096] {};
 for ( unsigned i=0; i<4; ++i ) dValues[i]=uint8_t(uRows>>(i*8));
 unsigned uBytes = 4;
 for ( uint32_t uRow=0; uRow<uRows; ++uRow )
 {
  uint64_t uValue = 0;
  if ( !fnGet(uRow,uValue) ) return false;
  if ( uBytes+8>sizeof(dValues) ) { tHash.Update ( dValues, int(uBytes) ); uBytes=0; }
  for ( unsigned i=0; i<8; ++i ) dValues[uBytes+i]=uint8_t(uValue>>(i*8));
  uBytes += 8;
 }
 if ( uBytes ) tHash.Update ( dValues, int(uBytes) );
 tHash.Final ( dDigest );
 return true;
}
inline bool AddTrustedFileIdentity ( std::string & sKey, const std::string & sFilename )
{
#if !defined(_WIN32)
 struct stat tStat {};
 if ( stat ( sFilename.c_str(), &tStat ) || !S_ISREG(tStat.st_mode) ) return false;
#if defined(__APPLE__)
 const auto & tMTime = tStat.st_mtimespec;
#else
 const auto & tMTime = tStat.st_mtim;
#endif
 sKey += ":" + std::to_string(uint64_t(tStat.st_dev)) + ":" + std::to_string(uint64_t(tStat.st_ino))
  + ":" + std::to_string(uint64_t(tStat.st_size)) + ":" + std::to_string(int64_t(tMTime.tv_sec))
  + ":" + std::to_string(int64_t(tMTime.tv_nsec));
 return true;
#else
 (void)sKey; (void)sFilename; return false;
#endif
}
inline std::string TrustedGenerationKey ( const std::string & sPostings, const std::string & sDict, const std::string & sHits, uint64_t uSize, uint32_t uPayload, uint32_t uDict, uint32_t uHits )
{
 std::string sKey = std::to_string(uSize)+":"+std::to_string(uPayload)+":"+std::to_string(uDict)+":"+std::to_string(uHits);
 return AddTrustedFileIdentity(sKey,sPostings) && AddTrustedFileIdentity(sKey,sDict) && AddTrustedFileIdentity(sKey,sHits) ? sKey : std::string();
}
inline void MarkTrustedGeneration ( const std::string & sPostings, const std::string & sDict, const std::string & sHits, uint64_t uSize, uint32_t uPayload, uint32_t uDict, uint32_t uHits, uint32_t uPublicIDRows=0, const PublicIDDigest_t * pPublicIDs=nullptr )
{
 auto sKey = TrustedGenerationKey ( sPostings, sDict, sHits, uSize, uPayload, uDict, uHits );
 if ( sKey.empty() ) return;
 TrustedGeneration_t tGeneration;
 if ( pPublicIDs ) { tGeneration.m_uPublicIDRows=uPublicIDRows; tGeneration.m_dPublicIDs=*pPublicIDs; tGeneration.m_bHasPublicIDs=true; }
 std::lock_guard<std::mutex> tLock ( TrustedGenerationsMutex() ); TrustedGenerations().emplace ( std::move(sKey), std::move(tGeneration) );
}
inline bool ConsumeTrustedGeneration ( const std::string & sPostings, const std::string & sDict, const std::string & sHits, uint64_t uSize, uint32_t uPayload, uint32_t uDict, uint32_t uHits )
{
 auto sKey = TrustedGenerationKey ( sPostings, sDict, sHits, uSize, uPayload, uDict, uHits );
 if ( sKey.empty() ) return false;
 std::lock_guard<std::mutex> tLock ( TrustedGenerationsMutex() );
 auto & dGenerations = TrustedGenerations(); auto iGeneration = dGenerations.find(sKey);
 if ( iGeneration==dGenerations.end() ) return false;
 dGenerations.erase(iGeneration); return true;
}
inline TrustedGenerationProof_t ConsumeTrustedGeneration ( const std::string & sPostings, const std::string & sDict, const std::string & sHits, uint64_t uSize, uint32_t uPayload, uint32_t uDict, uint32_t uHits, uint32_t uPublicIDRows, const PublicIDDigest_t * pPublicIDs )
{
 auto sKey = TrustedGenerationKey ( sPostings, sDict, sHits, uSize, uPayload, uDict, uHits );
 if ( sKey.empty() ) return {};
 std::lock_guard<std::mutex> tLock ( TrustedGenerationsMutex() );
 auto & dGenerations = TrustedGenerations(); auto iGeneration = dGenerations.find(sKey);
 if ( iGeneration==dGenerations.end() ) return {};
 TrustedGenerationProof_t tResult { true, pPublicIDs && iGeneration->second.m_bHasPublicIDs && iGeneration->second.m_uPublicIDRows==uPublicIDRows && iGeneration->second.m_dPublicIDs==*pPublicIDs };
 dGenerations.erase(iGeneration);
 return tResult;
}
inline uint32_t U32(const uint8_t *p) { return uint32_t(p[0]) | uint32_t(p[1])<<8 | uint32_t(p[2])<<16 | uint32_t(p[3])<<24; }
inline uint64_t U64(const uint8_t *p) { return U32(p) | uint64_t(U32(p+4))<<32; }
inline const std::array<uint32_t,256> & CRCTable() {
 static const auto dTable=[] {
  std::array<uint32_t,256> dResult{};
  for(uint32_t i=0;i<256;++i){uint32_t c=i;for(int j=0;j<8;++j)c=(c>>1)^(0xedb88320u&(0u-(c&1)));dResult[i]=c;}
  return dResult;
 }();
 return dTable;
}
inline const std::array<std::array<uint32_t,256>,8> & CRCSliceTables() {
 static const auto dTables=[] {
  std::array<std::array<uint32_t,256>,8> dResult{};
  dResult[0]=CRCTable();
  for(unsigned iSlice=1;iSlice<8;++iSlice)
   for(unsigned i=0;i<256;++i) { const uint32_t c=dResult[iSlice-1][i]; dResult[iSlice][i]=(c>>8)^dResult[0][c&255u]; }
  return dResult;
 }();
 return dTables;
}
inline uint32_t CRCUpdate(uint32_t c,const uint8_t *p,size_t n) {
 const auto &dTables=CRCSliceTables();
 while(n>=8) {
  c^=U32(p);
  c=dTables[7][c&255u]^dTables[6][(c>>8)&255u]^dTables[5][(c>>16)&255u]^dTables[4][c>>24]
   ^dTables[3][p[4]]^dTables[2][p[5]]^dTables[1][p[6]]^dTables[0][p[7]];
  p+=8; n-=8;
 }
 while(n--) c=dTables[0][(c^*p++)&255u]^(c>>8);
 return c;
}
inline uint32_t CRC(const uint8_t *p,size_t n) { return ~CRCUpdate(~0u,p,n); }
inline bool Var(const uint8_t *p, uint64_t size, uint64_t &off, uint64_t &v) {
 v=0; for(int i=0;i<10;++i) { if(off>=size || v>(UINT64_MAX>>7)) return false; auto b=p[off++]; v=(v<<7)|(b&127); if(!(b&128)) return true; } return false;
}
// v2 descriptor flags: codec[0:3], TF[4:5], mask[6:7], inline32[8], gap width[16:21].
// TF: one/constant/raw; mask: one/constant/raw/zero. Low32 is never a full-field claim.
inline uint32_t StreamBytes(uint32_t mode,uint32_t n) { return mode==1 ? 4 : mode==2 ? n*4 : 0; }
inline uint32_t StreamValue(const uint8_t *p,uint32_t mode,uint32_t i) { return mode==0 ? 1 : mode==3 ? 0 : U32(p+(mode==2 ? i*4 : 0)); }
inline uint64_t MetaBytes(uint32_t flags,uint32_t n,bool compact) {
 return compact ? StreamBytes((flags>>4)&3,n)+StreamBytes((flags>>6)&3,n)+uint64_t(n)*(flags&256 ? 4 : 8) : uint64_t(n)*16;
}
inline void Metadata(const uint8_t *p,uint32_t flags,uint32_t n,uint32_t i,bool compact,uint32_t &tf,uint32_t &mask,uint64_t &ref) {
 if(!compact) {tf=U32(p+i*4);mask=U32(p+n*4+i*4);ref=U64(p+n*8+i*8);return;}
 auto tm=(flags>>4)&3,mm=(flags>>6)&3;
 tf=StreamValue(p,tm,i);p+=StreamBytes(tm,n);mask=StreamValue(p,mm,i);p+=StreamBytes(mm,n);
 ref=flags&256 ? (uint64_t(1)<<63)|U32(p+i*4) : U64(p+i*8);
}
// E1/4 metadata: bounded ordinal reads, unsigned min-base refs, no delta chain.
inline uint64_t Packed(const uint8_t*p,unsigned width,unsigned i) {
 uint64_t out=0;uint64_t bit=uint64_t(i)*width;unsigned done=0;
 while(done<width){unsigned shift=bit%8,take=std::min(8-shift,width-done);out|=uint64_t((p[bit/8]>>shift)&((1u<<take)-1))<<done;done+=take;bit+=take;}return out;
}
// A wide read is legal only inside this packed stream, never in its padding.
inline uint64_t PackedBounded(const uint8_t*p,unsigned width,unsigned i,unsigned n) {
 if(!width)return 0;
 uint64_t bit=uint64_t(i)*width,offset=bit/8,bytes=(uint64_t(n)*width+7)/8;
 if(bytes-offset<8)return Packed(p,width,i);
 unsigned shift=bit%8;uint64_t value=U64(p+offset)>>shift;
 if(width+shift>64)value|=uint64_t(p[offset+8])<<(64-shift);
 return value&((uint64_t(1)<<width)-1);
}
template<typename T>
inline void UnpackBlock(const uint8_t *p,unsigned width,uint32_t n,T *out) {
	if(!width){std::fill_n(out,n,T(0));return;}
#if defined(__SIZEOF_INT128__)
	unsigned __int128 bits=0;
	unsigned have=0;
	const uint64_t mask=width==64?UINT64_MAX:(uint64_t(1)<<width)-1;
	for(uint32_t i=0;i<n;++i){
		while(have<width){bits|=static_cast<unsigned __int128>(*p++)<<have;have+=8;}
		out[i]=T(uint64_t(bits)&mask);bits>>=width;have-=width;
	}
#else
	for(uint32_t i=0;i<n;++i)out[i]=T(PackedBounded(p,width,i,n));
#endif
}
inline uint64_t Meta4Bytes(uint32_t f,uint32_t n) {
 auto tm=(f>>4)&3,mm=(f>>6)&3,tw=(f>>9)&63,rw=(f>>16)&63,mw=(f>>23)&63;
 return (tm==2?(uint64_t(n)*tw+7)/8:StreamBytes(tm,n))+(mm==2&&mw?(uint64_t(n)*mw+7)/8:StreamBytes(mm,n))+8+(uint64_t(n)*rw+7)/8;
}
inline bool Metadata4(const uint8_t*p,uint32_t f,uint32_t n,uint32_t i,uint32_t&tf,uint32_t&mask,uint64_t&ref) {
 auto tm=(f>>4)&3,mm=(f>>6)&3,tw=(f>>9)&63,rw=(f>>16)&63,mw=(f>>23)&63;bool split=f&(1u<<29);
 tf=tm==2?uint32_t(PackedBounded(p,tw,i,n)):StreamValue(p,tm,i);p+=tm==2?(uint64_t(n)*tw+7)/8:StreamBytes(tm,n);
 mask=mm==2&&mw?uint32_t(PackedBounded(p,mw,i,n)):StreamValue(p,mm,i);p+=mm==2&&mw?(uint64_t(n)*mw+7)/8:StreamBytes(mm,n);auto base=U64(p),delta=PackedBounded(p+8,rw,i,n);
 if(split&&(f&(1u<<22))&&tf==1){if(!mask||(mask&(mask-1))||delta>0x00ffffff)return false;base=(delta>>1)|((delta&1u)<<23)|(uint64_t(__builtin_ctz(mask))<<24);delta=0;}
 if(base>uint64_t(INT64_MAX)||delta>uint64_t(INT64_MAX)-base)return false;ref=base+delta;
 if((f&(1u<<22))&&tf==1)ref|=uint64_t(1)<<63;return true;
}
// Refill only the requested metadata cache. Descriptor/stream setup is per block;
// PackedBounded retains exact stream-tail and ninth-byte safety.
inline bool Metadata4Block(const uint8_t*p,uint32_t f,uint32_t n,uint32_t*tf,uint32_t*mask,uint64_t*ref) {
 auto tm=(f>>4)&3,mm=(f>>6)&3,tw=(f>>9)&63,rw=(f>>16)&63,mw=(f>>23)&63;bool split=f&(1u<<29);
 if(tm==2) UnpackBlock(p,tw,n,tf);
 else std::fill_n(tf,n,StreamValue(p,tm,0));
 p+=tm==2?(uint64_t(n)*tw+7)/8:StreamBytes(tm,n);
 if(mm==2&&mw) UnpackBlock(p,mw,n,mask);
 else if(mm==2) {for(uint32_t i=0;i<n;++i)mask[i]=U32(p+i*4);}
 else std::fill_n(mask,n,StreamValue(p,mm,0));
 p+=mm==2&&mw?(uint64_t(n)*mw+7)/8:StreamBytes(mm,n);auto base=U64(p);p+=8;
 if(base>uint64_t(INT64_MAX))return false;
 auto limit=uint64_t(INT64_MAX)-base;
 UnpackBlock(p,rw,n,ref);
 if(f&(1u<<22)) {
  for(uint32_t i=0;i<n;++i){auto delta=ref[i],itemBase=base;if(split&&tf[i]==1){if(!mask[i]||(mask[i]&(mask[i]-1))||delta>0x00ffffff)return false;itemBase=(delta>>1)|((delta&1u)<<23)|(uint64_t(__builtin_ctz(mask[i]))<<24);delta=0;}if(itemBase>uint64_t(INT64_MAX)||delta>uint64_t(INT64_MAX)-itemBase)return false;ref[i]=(itemBase+delta)|(tf[i]==1?uint64_t(1)<<63:0);}
 } else {
  for(uint32_t i=0;i<n;++i){auto delta=ref[i];if(delta>limit)return false;ref[i]=base+delta;}
 }
 return true;
}
// Caller validates descriptor and exact payload bounds before decoding. Never reads padding.
inline bool DecodeRows(const uint8_t *p,uint32_t first,uint32_t last,uint32_t n,uint32_t flags,bool compact,uint32_t *out) {
 auto codec=compact ? flags&15 : flags;
 if(codec==1) { if(uint64_t(first)+n-1!=last)return false;for(uint32_t i=0;i<n;++i)out[i]=first+i; }
 else if(codec==2) {for(uint32_t i=0;i<n;++i)out[i]=compact ? (i ? U32(p+(i-1)*4) : first) : U32(p+i*4);}
 else if(codec==3) {
  out[0]=first;uint32_t width=(flags>>16)&63;uint64_t bits=0;unsigned have=0;
  for(uint32_t i=1;i<n;++i) {
   while(have<width){bits|=uint64_t(*p++)<<have;have+=8;}
   uint32_t gap=uint32_t(bits&((uint64_t(1)<<width)-1));bits>>=width;have-=width;
   if(!gap || uint64_t(out[i-1])+gap>last)return false;out[i]=out[i-1]+gap;
  }
  if(bits)return false; // canonical unused high bits
 } else if(codec==4) {
  uint32_t span=last-first+1,j=0;
  for(uint32_t k=0;k<(span+7)/8;++k){unsigned byte=p[k];while(byte){unsigned bit=__builtin_ctz(byte);uint32_t delta=k*8+bit;if(delta>=span || j>=n)return false;out[j++]=first+delta;byte&=byte-1;}}
  if(j!=n)return false;
 } else return false;
 for(uint32_t i=0;i<n;++i)if((!i && out[i]!=first)||(i && out[i]<=out[i-1])||out[i]>last)return false;
 return out[n-1]==last;
}
}
