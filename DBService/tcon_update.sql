DROP PROCEDURE IF EXISTS tcon_update

DELIMITER //
CREATE PROCEDURE tcon_update(IN name_str VARCHAR(200), IN md5_str VARCHAR(32), IN content_value LONGTEXT, IN offset_value INT, IN size_value INT, IN author_str VARCHAR(50))
BEGIN
    IF EXISTS(SELECT md5 FROM tv4th.tcon_hstvt WHERE md5 = md5_str) THEN
        IF EXISTS(SELECT md5 FROM tv4th.tcon_testplan WHERE md5 = md5_str) THEN
            UPDATE tv4th.tcon_testplan SET editor = author_str WHERE md5 = md5_str;
        ELSE
            INSERT INTO tv4th.tcon_testplan(name, md5, offset, size, creator) VALUES(name_str, md5_str, offset_value, size_value, author_str);
        END IF;
    ELSE
        INSERT INTO tv4th.tcon_hstvt(md5, content) VALUES(md5_str, content_value);
        INSERT INTO tv4th.tcon_testplan(name, md5, offset, size, creator) VALUES(name_str, md5_str, offset_value, size_value, author_str);
    END IF;
END //
DELIMITER ;
