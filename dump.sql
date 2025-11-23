-- MySQL dump 10.13  Distrib 9.1.0, for Linux (x86_64)
--
-- Host: manticore    Database: manticore
-- ------------------------------------------------------
-- Server version	0.0.0
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `t`
--

DROP TABLE IF EXISTS `t`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `t` (
`id` bigint,
`f` text,
`a` integer,
`b` float,
`j` json,
`m` multi,
`s` string attribute,
`e` bool,
`d` timestamp,
`v` multi64,
`fv` float_vector
);
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `t`
--

LOCK TABLES `t` WRITE;
/*!40000 ALTER TABLE `t` DISABLE KEYS */;
INSERT INTO `t` VALUES ('1','',0,0.000000,'',(),'',0,0,(),()),('2','\'',0,0.000000,'',(),'',0,0,(),()),('3','\"',0,0.000000,'',(),'',0,0,(),()),('4','',0,0.000000,'',(),'\'',0,0,(),()),('5','',0,0.000000,'',(),'\"',0,0,(),()),('6','',0,0.000000,'{"a":"\'","b":"","c":"\'"}',(),'',0,0,(),()),('7','',0,0.000000,'{"a":"\\"","b":"\\"","c":"\\""}',(),'',0,0,(),()),('8','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',139090,912626112.000000,'{"a":[2140855600,1509344502],"b":1114121522}',(928973100,1245670504,1413286320),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',1,538847777,(1617199777),(1258482176.000000)),('9','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',198907,8841452.000000,'{"a":[21374549,321067876],"b":333059460}',(1274214921,1499886331,1977497504),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',1,782785729,(954464984),(696490880.000000)),('10','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',91421,1982302976.000000,'{"a":[1039865880,1000430726],"b":900846479}',(427690075,1407612830,1714474875),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',1,1649242630,(308109277),(27668260.000000)),('11','bbbbbbbbbbbbbbbbbb',121311,121581552.000000,'{"a":[1820436984,1682929863],"b":1502056736}',(1581765396,1654516653,1925375373),'bbbbbbbbbbbbbbbbbb',1,801766056,(1127147375),(107321288.000000)),('12','bbbbbbbbbbbbbb',191122,1229779968.000000,'{"a":[372959725,1851192973],"b":1660390017}',(185371603,608681734,1167859658),'bbbbbbbbbbbbbb',1,1213118476,(2124217000),(406914176.000000)),('13','bbbbbbbbbb',95482,898241728.000000,'{"a":[272656722,1663615544],"b":1624186067}',(116478871,552875970,1092517015),'bbbbbbbbbb',0,1239989597,(551802408),(152522160.000000)),('14','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',68119,661707200.000000,'{"a":[94304443,2086306634],"b":543066914}',(578274552,1237733450,2002653905),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',1,1710306734,(167580149),(2017031680.000000)),('15','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',32982,1377983488.000000,'{"a":[496823143,416865286],"b":91350609}',(1599607162,1936289567,2053679064),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',0,1518500442,(1228024866),(1438869760.000000)),('16','bbbb',127656,144601936.000000,'{"a":[407695664,2063437019],"b":917328502}',(645985148,1065653647,1744659471),'bbbb',0,314320885,(1525544836),(219136976.000000)),('17','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',42017,1793538432.000000,'{"a":[813865368,1069542460],"b":1420434902}',(1064502376,1165315561,1267507259),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',1,284197670,(454491894),(202294912.000000)),('18','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',187471,1981036928.000000,'{"a":[364450401,1780265560],"b":754576639}',(107468665,411366113,991124073),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',0,245327580,(1944713760),(1201735552.000000)),('19','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',32712,1558907136.000000,'{"a":[1912418574,1670100966],"b":239810528}',(24970160,1819190943,2021816520),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',1,1341382959,(283037034),(1458644736.000000)),('20','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',48474,62408120.000000,'{"a":[1200217209,500666982],"b":2116623760}',(328478084,504655829,1689585645),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',1,920211881,(229395197),(1011787776.000000)),('21','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',73513,303434976.000000,'{"a":[1896597632,617357533],"b":426659202}',(1756115916,2085689322,2094009064),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',1,635611892,(49226090),(2048176896.000000)),('22','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',183417,409343456.000000,'{"a":[2067015508,386772979],"b":771917505}',(479023156,814388870,1426741374),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',0,262452108,(1625607110),(2136056832.000000)),('23','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',189770,2015627264.000000,'{"a":[1008784233,1060446236],"b":1346785576}',(41530432,1122171594,1946302254),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',1,1659778950,(1695762450),(1899763200.000000)),('24','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',195315,1930263552.000000,'{"a":[1909437481,1372601335],"b":1730043281}',(471934518,529627550,1248685796),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',0,1085497807,(462381709),(1286633472.000000)),('25','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',149421,1482707968.000000,'{"a":[366358004,1989852354],"b":933978474}',(624130189,1827235823,2040641084),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',0,712631609,(1056904730),(224204880.000000)),('26','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',199699,1979034752.000000,'{"a":[139259276,1879105891],"b":1472314948}',(353482697,663275185,1574300218),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',0,1195294149,(994209483),(2033352448.000000)),('27','bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',122710,1789460480.000000,'{"a":[1663673360,654377261],"b":783359973}',(154668841,1084965318,1431522920),'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',0,685067488,(1750408871),(2004344960.000000));
/*!40000 ALTER TABLE `t` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2025-11-23 23:13:20
=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2025-11-23 23:13:20
