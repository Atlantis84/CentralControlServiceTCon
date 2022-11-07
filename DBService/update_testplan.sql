DROP PROCEDURE IF EXISTS update_testplan;

DELIMITER //
CREATE PROCEDURE update_testplan(IN proj_str VARCHAR(1), IN board_str VARCHAR(200), IN bom_str VARCHAR(200), IN st INT, IN author_str VARCHAR(50))
BEGIN
	DECLARE id_v BIGINT;
	DECLARE uuid_v VARCHAR(200);
	
	SELECT uuid INTO uuid_v FROM tv4th.testplan WHERE project = proj_str AND board = board_str AND bom = bom_str;
	IF uuid_v IS NOT NULL THEN
		IF st != -1 THEN
			UPDATE tv4th.testplan SET stat = st, editor = author_str WHERE uuid = uuid_v;
		END IF;
	ELSE
	    IF st != -1 THEN
		   	INSERT INTO tv4th.testplan(uuid, project, board, bom, stat, creator) VALUES(UPPER(UUID()), proj_str, board_str, bom_str, st, author_str);
		ELSE
		   	INSERT INTO tv4th.testplan(uuid, project, board, bom, stat, creator) VALUES(UPPER(UUID()), proj_str, board_str, bom_str, 1, author_str);
		END IF;
	END IF;

	if uuid_v IS NULL THEN
		SELECT MAX(id) INTO id_v FROM tv4th.testplan;
		SELECT uuid INTO uuid_v FROM tv4th.testplan WHERE id = id_v;
	END IF;
	SELECT uuid_v;
END //

DELIMITER ;

DROP PROCEDURE IF EXISTS upload_testplan;

DELIMITER //
CREATE PROCEDURE upload_testplan(IN uuid_str VARCHAR(50), IN md5_str VARCHAR(200), IN md5_context LONGTEXT, IN author_str VARCHAR(50))
BEGIN
	DECLARE id_v BIGINT;
	DECLARE uuid_v VARCHAR(200);
	
	SELECT uuid INTO uuid_v FROM tv4th.testplan WHERE project = proj_str AND board = board_str AND bom = bom_str;
	IF uuid_v IS NOT NULL THEN
		UPDATE tv4th.testplan SET editor = author_str WHERE uuid = uuid_v;
	ELSE 
		IF st != -1 THEN
			INSERT INTO tv4th.testplan(uuid, project, board, bom, stat, creator) VALUES(UPPER(UUID()), proj_str, board_str, bom_str, st, author_str);
		END IF;
	END IF;

	if uuid_v IS NULL THEN
		SELECT MAX(id) INTO id_v FROM tv4th.testplan;
		SELECT uuid INTO uuid_v FROM tv4th.testplan WHERE id = id_v;
	END IF;
	SELECT uuid_v;
END //

DELIMITER ;
