/*
SQLyog Enterprise - MySQL GUI v5.20
Host - 5.0.27-community-nt : Database - sphinx_test
*********************************************************************
Server version : 5.0.27-community-nt
*/

SET NAMES utf8;

SET SQL_MODE='';

CREATE database IF NOT EXISTS `sphinx_test`;

USE `sphinx_test`;

/* Table structure for table `links` */

DROP TABLE IF EXISTS `links`;

CREATE TABLE `links` (
  `id` INT(11) NOT NULL auto_increment,
  `name` VARCHAR(255) NOT NULL,
  `created_at` DATETIME NOT NULL,
  `description` TEXT,
  `group_id` INT(11) NOT NULL,
  `rating` FLOAT NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/* Table structure for table `tags` */

DROP TABLE IF EXISTS `tags`;

CREATE TABLE `tags` (
  `id` INT(11) NOT NULL auto_increment,
  `tag` VARCHAR(255) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/* Table structure for table `links_tags` */

DROP TABLE IF EXISTS `links_tags`;

CREATE TABLE `links_tags` (
  `link_id` INT(11) NOT NULL,
  `tag_id` INT(11) NOT NULL,
  PRIMARY KEY  (`link_id`,`tag_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/* Table structure for table `links64` */

DROP TABLE IF EXISTS `links64`;

CREATE TABLE `links64` (
  `id` BIGINT(11) NOT NULL auto_increment,
  `name` VARCHAR(255) NOT NULL,
  `created_at` DATETIME NOT NULL,
  `description` TEXT,
  `group_id` INT(11) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/* Data for the table `links` */

INSERT INTO `links`(`id`,`name`,`created_at`,`description`,`group_id`,`rating`) VALUES
	(1,'Paint Protects WiFi Network from Hackers','2007-04-04 06:48:10','A company known as SEC Technologies has created a special type of paint that blocks Wi-Fi signals so that you can be sure hackers can ',1,13.32),
	(2,'Airplanes To Become WiFi Hotspots','2007-04-04 06:49:15','Airlines will start turning their airplanes into WiFi hotspots beginning early next year, WSJ reports. Here\'s what you need to know...',2,54.85),
	(3,'Planet VIP-195 GSM/WiFi Phone With Windows Messanger','2007-04-04 06:50:47','The phone does comply with IEEE 802.11b and IEEE 802.11g to provide phone capability via WiFi. As GSM phone the VIP-195 support 900/1800/1900 band and GPRS too. It comes with simple button to switch between WiFi or GSM mod',1,16.25);

/* Data for the table `tags` */
INSERT INTO `tags`(`id`,`tag`) VALUES
	(1, 'tag1'),(2, 'tag2'),(3, 'tag3'),(4, 'tag4'),(5, 'tag5'),
	(6, 'tag6'),(7, 'tag7'),(8, 'tag8'),(9, 'tag9'),(10, 'tag5');

/* Data for the table `links_tags` */
INSERT INTO `links_tags`(`link_id`,`tag_id`) VALUES
	(1, 1),(1, 2),(1, 3),(1, 4),
	(2, 5),(2, 6),(2, 7),(2, 8),
	(3, 9),(3, 1),(3, 7),(3, 10);
	
/* Data for the table `links64` */

INSERT INTO `links64`(`id`,`name`,`created_at`,`description`,`group_id`) VALUES
	(4294967297,'Paint Protects WiFi Network from Hackers','2007-04-04 06:48:10','A company known as SEC Technologies has created a special type of paint that blocks Wi-Fi signals so that you can be sure hackers can ',1),
	(4294967298,'Airplanes To Become WiFi Hotspots','2007-04-04 06:49:15','Airlines will start turning their airplanes into WiFi hotspots beginning early next year, WSJ reports. Here\'s what you need to know...',2),
	(4294967299,'Planet VIP-195 GSM/WiFi Phone With Windows Messanger','2007-04-04 06:50:47','The phone does comply with IEEE 802.11b and IEEE 802.11g to provide phone capability via WiFi. As GSM phone the VIP-195 support 900/1800/1900 band and GPRS too. It comes with simple button to switch between WiFi or GSM mod',1);
