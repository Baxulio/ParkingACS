-- --------------------------------------------------------
-- Хост:                         localhost
-- Версия сервера:               5.7.16 - MySQL Community Server (GPL)
-- Операционная система:         Win32
-- HeidiSQL Версия:              9.5.0.5196
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- Дамп структуры базы данных Attraction
CREATE DATABASE IF NOT EXISTS `Attraction` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `Attraction`;

-- Дамп структуры для таблица Attraction.active_bracers
CREATE TABLE IF NOT EXISTS `active_bracers` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `code` int(11) NOT NULL,
  `bracer_number` smallint(5) unsigned NOT NULL,
  `enter_time` datetime DEFAULT NULL,
  `enter_number` tinyint(4) DEFAULT NULL,
  `deposit_id` int(11) NOT NULL,
  `tariff_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `FK_active_bracers_deposit` (`deposit_id`),
  KEY `FK_active_bracers_tariff` (`tariff_id`),
  CONSTRAINT `FK_active_bracers_deposit` FOREIGN KEY (`deposit_id`) REFERENCES `deposit` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `FK_active_bracers_tariff` FOREIGN KEY (`tariff_id`) REFERENCES `tariff` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Экспортируемые данные не выделены.
-- Дамп структуры для таблица Attraction.active_transactions
CREATE TABLE IF NOT EXISTS `active_transactions` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `product_id` int(11) NOT NULL,
  `quantity` int(11) NOT NULL DEFAULT '1',
  `total_price` decimal(10,2) NOT NULL,
  `time` datetime NOT NULL,
  `active_bracers_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `FK_active_transactions_active_bracers` (`active_bracers_id`),
  KEY `FK_active_transactions_products` (`product_id`),
  CONSTRAINT `FK_active_transactions_active_bracers` FOREIGN KEY (`active_bracers_id`) REFERENCES `active_bracers` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `FK_active_transactions_products` FOREIGN KEY (`product_id`) REFERENCES `products` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Экспортируемые данные не выделены.
-- Дамп структуры для таблица Attraction.bracers_history
CREATE TABLE IF NOT EXISTS `bracers_history` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `code` int(11) NOT NULL,
  `bracer_number` smallint(5) unsigned NOT NULL,
  `enter_time` datetime NOT NULL,
  `enter_number` tinyint(4) NOT NULL,
  `exit_time` datetime NOT NULL,
  `exit_number` tinyint(4) NOT NULL,
  `cash` decimal(10,2) NOT NULL,
  `tariff_price` decimal(10,2) NOT NULL,
  `tariff_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `FK_bracers_history_tariff` (`tariff_id`),
  CONSTRAINT `FK_bracers_history_tariff` FOREIGN KEY (`tariff_id`) REFERENCES `tariff` (`id`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

-- Экспортируемые данные не выделены.
-- Дамп структуры для таблица Attraction.deposit
CREATE TABLE IF NOT EXISTS `deposit` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `cash` decimal(10,2) NOT NULL DEFAULT '0.00',
  `expired` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Экспортируемые данные не выделены.
-- Дамп структуры для таблица Attraction.products
CREATE TABLE IF NOT EXISTS `products` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `title` varchar(50) NOT NULL,
  `price` decimal(10,2) NOT NULL DEFAULT '0.00',
  `product_types_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `FK_products_product_types` (`product_types_id`),
  CONSTRAINT `FK_products_product_types` FOREIGN KEY (`product_types_id`) REFERENCES `product_types` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Экспортируемые данные не выделены.
-- Дамп структуры для таблица Attraction.product_types
CREATE TABLE IF NOT EXISTS `product_types` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `title` varchar(50) NOT NULL,
  `mode` enum('Park','Bar') NOT NULL DEFAULT 'Park',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Экспортируемые данные не выделены.
-- Дамп структуры для таблица Attraction.tariff
CREATE TABLE IF NOT EXISTS `tariff` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `title` varchar(50) NOT NULL,
  `price` decimal(10,2) NOT NULL DEFAULT '0.00',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Экспортируемые данные не выделены.
-- Дамп структуры для таблица Attraction.transactions_history
CREATE TABLE IF NOT EXISTS `transactions_history` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `product_id` int(11) NOT NULL DEFAULT '1',
  `quantity` int(11) NOT NULL DEFAULT '1',
  `total_price` decimal(10,2) NOT NULL,
  `time` datetime NOT NULL,
  `bracers_history_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `FK_active_transactions_active_bracers` (`bracers_history_id`),
  KEY `FK_transactions_history_products` (`product_id`),
  CONSTRAINT `FK_transactions_history_products` FOREIGN KEY (`product_id`) REFERENCES `products` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `transactions_history_ibfk_1` FOREIGN KEY (`bracers_history_id`) REFERENCES `bracers_history` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

-- Экспортируемые данные не выделены.
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
