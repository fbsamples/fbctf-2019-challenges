SET GLOBAL event_scheduler = ON;
USE fbctf;

CREATE TABLE products (
name char(64),
secret char(64),
description varchar(250)
);

DELIMITER //

CREATE PROCEDURE init_contents_proc()
  LANGUAGE SQL
  BEGIN
    INSERT INTO products VALUES('facebook','824e632f708e1de6f9c0134be41a0fcd8b7ae42d34805e38649f8f9fe78e9bab','Facebook, Inc. is an American online social media and social networking service company based in Menlo Park, California. Very cool! Here is a flag for you: fb{4774ck1n9_5q1_w17h0u7_1nj3c710n_15_4m421n9_:)}');

    INSERT INTO products VALUES('messenger','52efe53c758291e58f1f1c12feaae12cd9b181cc53e3fe92effe380085146f2d','Facebook Messenger is a messaging app and platform. Originally developed as Facebook Chat in 2008, the company revamped its messaging service in 2010, and subsequently released standalone iOS and Android apps in August 2011');

    INSERT INTO products VALUES('instagram','66bf09edef42fc4efac13239fda39a91f0d5b85ae96ec436b7dded4104e7b96d','Instagram is a photo and video-sharing social networking service owned by Facebook, Inc. It was created by Kevin Systrom and Mike Krieger, and launched in October 2010 exclusively on iOS');

    INSERT INTO products VALUES('whatsapp','8096a17b6482c25fddd690a9c2bab328d100e3a841b3b067fd43cf98c841cb03','WhatsApp Messenger is a freeware, cross-platform messaging and Voice over IP service owned by Facebook. It allows the sending of text messages and voice calls, as well as video calls, images and other media, documents, and user location.');

    INSERT INTO products VALUES('oculus-rift','d97ba6bd84d6abd896c5572b66f2ad865f8fb66c0d852c18c2e7f678135bfc02','Oculus Rift is a virtual reality headset developed and manufactured by Oculus VR, a division of Facebook Inc., released on March 28, 2016.');
  END //

CREATE EVENT cronjob
  ON SCHEDULE EVERY 2 MINUTE
  DO
    BEGIN
      TRUNCATE TABLE products;
      CALL init_contents_proc();
    END //

DELIMITER ;

