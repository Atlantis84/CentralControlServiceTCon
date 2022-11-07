DROP PROCEDURE IF EXISTS tcon_check_upgrade;

DELIMITER //
CREATE PROCEDURE tcon_check_upgrade(IN type VARCHAR(200), IN area VARCHAR(200), IN machine VARCHAR(200), IN board VARCHAR(200), IN machine_sytle VARCHAR(200), IN half_material_no VARCHAR(200), IN st INT)
BEGIN
    DECLARE stat_v INT;
    DECLARE typeName_v VARCHAR(200);
    DECLARE areaName_v VARCHAR(200);
    DECLARE machineName_v VARCHAR(200);
    DECLARE boardStyle_v VARCHAR(200);
    DECLARE machineStyleName_v VARCHAR(200);
    DECLARE halfMaterialNumber_v VARCHAR(200);
    DECLARE id_v BIGINT;

    SELECT id INTO id_v FROM tv4th.tcon_upgrade WHERE typeName = type AND areaName = area AND machineName = machine;
    IF id_v IS NOT NULL THEN
        SELECT boardStyle INTO boardStyle_v FROM tv4th.tcon_upgrade WHERE id = id_v;
        SELECT machineStyleName INTO machineStyleName_v FROM tv4th.tcon_upgrade WHERE id = id_v;
        SELECT halfMaterialNumber INTO halfMaterialNumber_v FROM tv4th.tcon_upgrade WHERE id = id_v;

        IF st != -1 THEN
            UPDATE tv4th.tcon_upgrade SET stat = st WHERE id = id_v;
        ELSE
            IF boardStyle_v != board OR machineStyleName_v != machine_sytle OR halfMaterialNumber_v != half_material_no THEN
                UPDATE tv4th.tcon_upgrade SET boardStyle = board, machineStyleName = machine_sytle, halfMaterialNumber = half_material_no, stat = 1 WHERE id = id_v;
            END IF;
        END IF;
    ELSE
        INSERT INTO tv4th.tcon_upgrade(typeName, areaName, machineName, boardStyle, machineStyleName, halfMaterialNumber, stat) VALUES(type, area, machine, board, machine_sytle, half_material_no, 1);
    END IF;

    SELECT areaName, machineName, boardStyle, machineStyleName, halfMaterialNumber, stat INTO areaName_v, machineName_v, boardStyle_v, machineStyleName_v, halfMaterialNumber_v, stat_v FROM tv4th.tcon_upgrade WHERE typeName = type AND areaName = area AND machineName = machine;
    SELECT areaName_v, machineName_v, boardStyle_v, machineStyleName_v, halfMaterialNumber_v, stat_v;
END //

DELIMITER ;
