SELECT MODEL1.is_mutagen, count(MODEL1.model_id ) FROM MODEL MODEL1, ATOM T1008290346560  WHERE MODEL1.model_id=T1008290346560.model_id group by MODEL1.is_mutagen;
SELECT MODEL2.is_mutagen, count(MODEL2.model_id ) FROM MODEL MODEL2, ATOM ATOM3, ATOM T1008290346670  WHERE MODEL2.model_id=ATOM3.model_id AND MODEL2.model_id=T1008290346670.model_id group by MODEL2.is_mutagen;
SELECT MODEL2.is_mutagen, count(MODEL2.model_id ) FROM MODEL MODEL2, ATOM ATOM3, BOND T1008290357490  WHERE MODEL2.model_id=ATOM3.model_id AND MODEL2.model_id=T1008290357490.model_id group by MODEL2.is_mutagen;
SELECT MODEL2.is_mutagen,MODEL2.is_mutagen, count(MODEL2.model_id ) FROM MODEL MODEL2, ATOM ATOM3  WHERE MODEL2.model_id=ATOM3.model_id group by MODEL2.is_mutagen , MODEL2.is_mutagen;
SELECT MODEL2.is_mutagen,MODEL2.lumo, count(MODEL2.model_id ) FROM MODEL MODEL2, ATOM ATOM3  WHERE MODEL2.model_id=ATOM3.model_id group by MODEL2.lumo , MODEL2.is_mutagen;
SELECT MODEL2.is_mutagen,MODEL2.logp, count(MODEL2.model_id ) FROM MODEL MODEL2, ATOM ATOM3  WHERE MODEL2.model_id=ATOM3.model_id group by MODEL2.logp , MODEL2.is_mutagen;
SELECT MODEL2.is_mutagen,ATOM3.element, count(MODEL2.model_id ) FROM MODEL MODEL2, ATOM ATOM3  WHERE MODEL2.model_id=ATOM3.model_id group by ATOM3.element , MODEL2.is_mutagen;
SELECT MODEL2.is_mutagen,ATOM3.type, count(MODEL2.model_id ) FROM MODEL MODEL2, ATOM ATOM3  WHERE MODEL2.model_id=ATOM3.model_id group by ATOM3.type , MODEL2.is_mutagen;
SELECT MODEL2.is_mutagen,ATOM3.charge, count(MODEL2.model_id ) FROM MODEL MODEL2, ATOM ATOM3  WHERE MODEL2.model_id=ATOM3.model_id group by ATOM3.charge , MODEL2.is_mutagen;
SELECT MODEL1.is_mutagen, count(MODEL1.model_id ) FROM MODEL MODEL1, BOND T1008290375670  WHERE MODEL1.model_id=T1008290375670.model_id group by MODEL1.is_mutagen;
SELECT MODEL110.is_mutagen, count(MODEL110.model_id ) FROM MODEL MODEL110, BOND BOND111, ATOM T1008290375830  WHERE MODEL110.model_id=BOND111.model_id AND MODEL110.model_id=T1008290375830.model_id group by MODEL110.is_mutagen;
SELECT MODEL110.is_mutagen, count(MODEL110.model_id ) FROM MODEL MODEL110, BOND BOND111, BOND T1008290387970  WHERE MODEL110.model_id=BOND111.model_id AND MODEL110.model_id=T1008290387970.model_id group by MODEL110.is_mutagen;
SELECT MODEL110.is_mutagen, count(MODEL110.model_id ) FROM MODEL MODEL110, BOND BOND111, ATOM T1008290400660  WHERE MODEL110.model_id=BOND111.model_id AND BOND111.atom1=T1008290400660.atom_id group by MODEL110.is_mutagen;
SELECT MODEL110.is_mutagen, count(MODEL110.model_id ) FROM MODEL MODEL110, BOND BOND111, ATOM T1008290401100  WHERE MODEL110.model_id=BOND111.model_id AND BOND111.atom2=T1008290401100.atom_id group by MODEL110.is_mutagen;
SELECT MODEL110.is_mutagen,MODEL110.is_mutagen, count(MODEL110.model_id ) FROM MODEL MODEL110, BOND BOND111  WHERE MODEL110.model_id=BOND111.model_id group by MODEL110.is_mutagen , MODEL110.is_mutagen;
SELECT MODEL110.is_mutagen,MODEL110.lumo, count(MODEL110.model_id ) FROM MODEL MODEL110, BOND BOND111  WHERE MODEL110.model_id=BOND111.model_id group by MODEL110.lumo , MODEL110.is_mutagen;
SELECT MODEL110.is_mutagen,MODEL110.logp, count(MODEL110.model_id ) FROM MODEL MODEL110, BOND BOND111  WHERE MODEL110.model_id=BOND111.model_id group by MODEL110.logp , MODEL110.is_mutagen;
SELECT MODEL110.is_mutagen,BOND111.type, count(MODEL110.model_id ) FROM MODEL MODEL110, BOND BOND111  WHERE MODEL110.model_id=BOND111.model_id group by BOND111.type , MODEL110.is_mutagen;
SELECT MODEL1.is_mutagen,MODEL1.is_mutagen, count(MODEL1.model_id ) FROM MODEL MODEL1  group by MODEL1.is_mutagen , MODEL1.is_mutagen;
SELECT MODEL162.is_mutagen, count(MODEL162.model_id ) FROM MODEL MODEL162, ATOM T1008290407030  WHERE MODEL162.model_id=T1008290407030.model_id AND MODEL162.is_mutagen='T' group by MODEL162.is_mutagen;
SELECT MODEL162.is_mutagen, count(MODEL162.model_id ) FROM MODEL MODEL162, BOND T1008290407360  WHERE MODEL162.model_id=T1008290407360.model_id AND MODEL162.is_mutagen='T' group by MODEL162.is_mutagen;
SELECT MODEL162.is_mutagen,MODEL162.is_mutagen, count(MODEL162.model_id ) FROM MODEL MODEL162  WHERE MODEL162.is_mutagen='T' group by MODEL162.is_mutagen , MODEL162.is_mutagen;
SELECT MODEL162.is_mutagen,MODEL162.lumo, count(MODEL162.model_id ) FROM MODEL MODEL162  WHERE MODEL162.is_mutagen='T' group by MODEL162.lumo , MODEL162.is_mutagen;
SELECT MODEL162.is_mutagen,MODEL162.logp, count(MODEL162.model_id ) FROM MODEL MODEL162  WHERE MODEL162.is_mutagen='T' group by MODEL162.logp , MODEL162.is_mutagen;
SELECT MODEL178.is_mutagen, count(MODEL178.model_id ) FROM MODEL MODEL178, ATOM T1008290410000  WHERE MODEL178.model_id=T1008290410000.model_id AND MODEL178.is_mutagen='F' group by MODEL178.is_mutagen;
SELECT MODEL178.is_mutagen, count(MODEL178.model_id ) FROM MODEL MODEL178, BOND T1008290410330  WHERE MODEL178.model_id=T1008290410330.model_id AND MODEL178.is_mutagen='F' group by MODEL178.is_mutagen;
SELECT MODEL178.is_mutagen,MODEL178.is_mutagen, count(MODEL178.model_id ) FROM MODEL MODEL178  WHERE MODEL178.is_mutagen='F' group by MODEL178.is_mutagen , MODEL178.is_mutagen;
SELECT MODEL178.is_mutagen,MODEL178.lumo, count(MODEL178.model_id ) FROM MODEL MODEL178  WHERE MODEL178.is_mutagen='F' group by MODEL178.lumo , MODEL178.is_mutagen;
SELECT MODEL178.is_mutagen,MODEL178.logp, count(MODEL178.model_id ) FROM MODEL MODEL178  WHERE MODEL178.is_mutagen='F' group by MODEL178.logp , MODEL178.is_mutagen;
SELECT MODEL1.is_mutagen,MODEL1.lumo, count(MODEL1.model_id ) FROM MODEL MODEL1  group by MODEL1.lumo , MODEL1.is_mutagen;
SELECT MODEL192.is_mutagen, count(MODEL192.model_id ) FROM MODEL MODEL192, ATOM T1008290412030  WHERE MODEL192.model_id=T1008290412030.model_id AND MODEL192.lumo='-3' group by MODEL192.is_mutagen;
SELECT MODEL192.is_mutagen, count(MODEL192.model_id ) FROM MODEL MODEL192, BOND T1008290412300  WHERE MODEL192.model_id=T1008290412300.model_id AND MODEL192.lumo='-3' group by MODEL192.is_mutagen;
SELECT MODEL192.is_mutagen,MODEL192.is_mutagen, count(MODEL192.model_id ) FROM MODEL MODEL192  WHERE MODEL192.lumo='-3' group by MODEL192.is_mutagen , MODEL192.is_mutagen;
SELECT MODEL192.is_mutagen,MODEL192.lumo, count(MODEL192.model_id ) FROM MODEL MODEL192  WHERE MODEL192.lumo='-3' group by MODEL192.lumo , MODEL192.is_mutagen;
SELECT MODEL192.is_mutagen,MODEL192.logp, count(MODEL192.model_id ) FROM MODEL MODEL192  WHERE MODEL192.lumo='-3' group by MODEL192.logp , MODEL192.is_mutagen;
SELECT MODEL202.is_mutagen, count(MODEL202.model_id ) FROM MODEL MODEL202, ATOM T1008290413400  WHERE MODEL202.model_id=T1008290413400.model_id AND MODEL202.lumo='-2' group by MODEL202.is_mutagen;
SELECT MODEL202.is_mutagen, count(MODEL202.model_id ) FROM MODEL MODEL202, BOND T1008290413680  WHERE MODEL202.model_id=T1008290413680.model_id AND MODEL202.lumo='-2' group by MODEL202.is_mutagen;
SELECT MODEL202.is_mutagen,MODEL202.is_mutagen, count(MODEL202.model_id ) FROM MODEL MODEL202  WHERE MODEL202.lumo='-2' group by MODEL202.is_mutagen , MODEL202.is_mutagen;
SELECT MODEL202.is_mutagen,MODEL202.lumo, count(MODEL202.model_id ) FROM MODEL MODEL202  WHERE MODEL202.lumo='-2' group by MODEL202.lumo , MODEL202.is_mutagen;
SELECT MODEL202.is_mutagen,MODEL202.logp, count(MODEL202.model_id ) FROM MODEL MODEL202  WHERE MODEL202.lumo='-2' group by MODEL202.logp , MODEL202.is_mutagen;
SELECT MODEL217.is_mutagen, count(MODEL217.model_id ) FROM MODEL MODEL217, ATOM T1008290417030  WHERE MODEL217.model_id=T1008290417030.model_id AND MODEL217.lumo='-1' group by MODEL217.is_mutagen;
SELECT MODEL217.is_mutagen, count(MODEL217.model_id ) FROM MODEL MODEL217, BOND T1008290417690  WHERE MODEL217.model_id=T1008290417690.model_id AND MODEL217.lumo='-1' group by MODEL217.is_mutagen;
SELECT MODEL217.is_mutagen,MODEL217.is_mutagen, count(MODEL217.model_id ) FROM MODEL MODEL217  WHERE MODEL217.lumo='-1' group by MODEL217.is_mutagen , MODEL217.is_mutagen;
SELECT MODEL217.is_mutagen,MODEL217.lumo, count(MODEL217.model_id ) FROM MODEL MODEL217  WHERE MODEL217.lumo='-1' group by MODEL217.lumo , MODEL217.is_mutagen;
SELECT MODEL217.is_mutagen,MODEL217.logp, count(MODEL217.model_id ) FROM MODEL MODEL217  WHERE MODEL217.lumo='-1' group by MODEL217.logp , MODEL217.is_mutagen;
SELECT MODEL1.is_mutagen,MODEL1.logp, count(MODEL1.model_id ) FROM MODEL MODEL1  group by MODEL1.logp , MODEL1.is_mutagen;
SELECT MODEL232.is_mutagen, count(MODEL232.model_id ) FROM MODEL MODEL232, ATOM T1008290420930  WHERE MODEL232.model_id=T1008290420930.model_id AND MODEL232.logp='8' group by MODEL232.is_mutagen;
SELECT MODEL232.is_mutagen, count(MODEL232.model_id ) FROM MODEL MODEL232, BOND T1008290421150  WHERE MODEL232.model_id=T1008290421150.model_id AND MODEL232.logp='8' group by MODEL232.is_mutagen;
SELECT MODEL232.is_mutagen,MODEL232.is_mutagen, count(MODEL232.model_id ) FROM MODEL MODEL232  WHERE MODEL232.logp='8' group by MODEL232.is_mutagen , MODEL232.is_mutagen;
SELECT MODEL232.is_mutagen,MODEL232.lumo, count(MODEL232.model_id ) FROM MODEL MODEL232  WHERE MODEL232.logp='8' group by MODEL232.lumo , MODEL232.is_mutagen;
SELECT MODEL232.is_mutagen,MODEL232.logp, count(MODEL232.model_id ) FROM MODEL MODEL232  WHERE MODEL232.logp='8' group by MODEL232.logp , MODEL232.is_mutagen;
SELECT MODEL237.is_mutagen, count(MODEL237.model_id ) FROM MODEL MODEL237, ATOM T1008290422250  WHERE MODEL237.model_id=T1008290422250.model_id AND MODEL237.logp='7' group by MODEL237.is_mutagen;
SELECT MODEL237.is_mutagen, count(MODEL237.model_id ) FROM MODEL MODEL237, BOND T1008290422850  WHERE MODEL237.model_id=T1008290422850.model_id AND MODEL237.logp='7' group by MODEL237.is_mutagen;
SELECT MODEL237.is_mutagen,MODEL237.is_mutagen, count(MODEL237.model_id ) FROM MODEL MODEL237  WHERE MODEL237.logp='7' group by MODEL237.is_mutagen , MODEL237.is_mutagen;
SELECT MODEL237.is_mutagen,MODEL237.lumo, count(MODEL237.model_id ) FROM MODEL MODEL237  WHERE MODEL237.logp='7' group by MODEL237.lumo , MODEL237.is_mutagen;
SELECT MODEL237.is_mutagen,MODEL237.logp, count(MODEL237.model_id ) FROM MODEL MODEL237  WHERE MODEL237.logp='7' group by MODEL237.logp , MODEL237.is_mutagen;
SELECT MODEL247.is_mutagen, count(MODEL247.model_id ) FROM MODEL MODEL247, ATOM T1008290427740  WHERE MODEL247.model_id=T1008290427740.model_id AND MODEL247.logp='6' group by MODEL247.is_mutagen;
SELECT MODEL247.is_mutagen, count(MODEL247.model_id ) FROM MODEL MODEL247, BOND T1008290427850  WHERE MODEL247.model_id=T1008290427850.model_id AND MODEL247.logp='6' group by MODEL247.is_mutagen;
SELECT MODEL247.is_mutagen,MODEL247.is_mutagen, count(MODEL247.model_id ) FROM MODEL MODEL247  WHERE MODEL247.logp='6' group by MODEL247.is_mutagen , MODEL247.is_mutagen;
SELECT MODEL247.is_mutagen,MODEL247.lumo, count(MODEL247.model_id ) FROM MODEL MODEL247  WHERE MODEL247.logp='6' group by MODEL247.lumo , MODEL247.is_mutagen;
SELECT MODEL247.is_mutagen,MODEL247.logp, count(MODEL247.model_id ) FROM MODEL MODEL247  WHERE MODEL247.logp='6' group by MODEL247.logp , MODEL247.is_mutagen;
SELECT MODEL255.is_mutagen, count(MODEL255.model_id ) FROM MODEL MODEL255, ATOM T1008290428950  WHERE MODEL255.model_id=T1008290428950.model_id AND MODEL255.logp='5' group by MODEL255.is_mutagen;
SELECT MODEL255.is_mutagen, count(MODEL255.model_id ) FROM MODEL MODEL255, BOND T1008290429280  WHERE MODEL255.model_id=T1008290429280.model_id AND MODEL255.logp='5' group by MODEL255.is_mutagen;
SELECT MODEL255.is_mutagen,MODEL255.is_mutagen, count(MODEL255.model_id ) FROM MODEL MODEL255  WHERE MODEL255.logp='5' group by MODEL255.is_mutagen , MODEL255.is_mutagen;
SELECT MODEL255.is_mutagen,MODEL255.lumo, count(MODEL255.model_id ) FROM MODEL MODEL255  WHERE MODEL255.logp='5' group by MODEL255.lumo , MODEL255.is_mutagen;
SELECT MODEL255.is_mutagen,MODEL255.logp, count(MODEL255.model_id ) FROM MODEL MODEL255  WHERE MODEL255.logp='5' group by MODEL255.logp , MODEL255.is_mutagen;
SELECT MODEL264.is_mutagen, count(MODEL264.model_id ) FROM MODEL MODEL264, ATOM T1008290434110  WHERE MODEL264.model_id=T1008290434110.model_id AND MODEL264.logp='4' group by MODEL264.is_mutagen;
SELECT MODEL264.is_mutagen, count(MODEL264.model_id ) FROM MODEL MODEL264, BOND T1008290434330  WHERE MODEL264.model_id=T1008290434330.model_id AND MODEL264.logp='4' group by MODEL264.is_mutagen;
SELECT MODEL264.is_mutagen,MODEL264.is_mutagen, count(MODEL264.model_id ) FROM MODEL MODEL264  WHERE MODEL264.logp='4' group by MODEL264.is_mutagen , MODEL264.is_mutagen;
SELECT MODEL264.is_mutagen,MODEL264.lumo, count(MODEL264.model_id ) FROM MODEL MODEL264  WHERE MODEL264.logp='4' group by MODEL264.lumo , MODEL264.is_mutagen;
SELECT MODEL264.is_mutagen,MODEL264.logp, count(MODEL264.model_id ) FROM MODEL MODEL264  WHERE MODEL264.logp='4' group by MODEL264.logp , MODEL264.is_mutagen;
SELECT MODEL275.is_mutagen, count(MODEL275.model_id ) FROM MODEL MODEL275, ATOM T1008290435540  WHERE MODEL275.model_id=T1008290435540.model_id AND MODEL275.logp='3' group by MODEL275.is_mutagen;
SELECT MODEL275.is_mutagen, count(MODEL275.model_id ) FROM MODEL MODEL275, BOND T1008290436030  WHERE MODEL275.model_id=T1008290436030.model_id AND MODEL275.logp='3' group by MODEL275.is_mutagen;
SELECT MODEL275.is_mutagen,MODEL275.is_mutagen, count(MODEL275.model_id ) FROM MODEL MODEL275  WHERE MODEL275.logp='3' group by MODEL275.is_mutagen , MODEL275.is_mutagen;
SELECT MODEL275.is_mutagen,MODEL275.lumo, count(MODEL275.model_id ) FROM MODEL MODEL275  WHERE MODEL275.logp='3' group by MODEL275.lumo , MODEL275.is_mutagen;
SELECT MODEL275.is_mutagen,MODEL275.logp, count(MODEL275.model_id ) FROM MODEL MODEL275  WHERE MODEL275.logp='3' group by MODEL275.logp , MODEL275.is_mutagen;
SELECT MODEL286.is_mutagen, count(MODEL286.model_id ) FROM MODEL MODEL286, ATOM T1008290437790  WHERE MODEL286.model_id=T1008290437790.model_id AND MODEL286.logp='2' group by MODEL286.is_mutagen;
SELECT MODEL286.is_mutagen, count(MODEL286.model_id ) FROM MODEL MODEL286, BOND T1008290438010  WHERE MODEL286.model_id=T1008290438010.model_id AND MODEL286.logp='2' group by MODEL286.is_mutagen;
SELECT MODEL286.is_mutagen,MODEL286.is_mutagen, count(MODEL286.model_id ) FROM MODEL MODEL286  WHERE MODEL286.logp='2' group by MODEL286.is_mutagen , MODEL286.is_mutagen;
SELECT MODEL286.is_mutagen,MODEL286.lumo, count(MODEL286.model_id ) FROM MODEL MODEL286  WHERE MODEL286.logp='2' group by MODEL286.lumo , MODEL286.is_mutagen;
SELECT MODEL286.is_mutagen,MODEL286.logp, count(MODEL286.model_id ) FROM MODEL MODEL286  WHERE MODEL286.logp='2' group by MODEL286.logp , MODEL286.is_mutagen;
SELECT MODEL297.is_mutagen, count(MODEL297.model_id ) FROM MODEL MODEL297, ATOM T1008290440150  WHERE MODEL297.model_id=T1008290440150.model_id AND MODEL297.logp='1' group by MODEL297.is_mutagen;
SELECT MODEL297.is_mutagen, count(MODEL297.model_id ) FROM MODEL MODEL297, BOND T1008290440700  WHERE MODEL297.model_id=T1008290440700.model_id AND MODEL297.logp='1' group by MODEL297.is_mutagen;
SELECT MODEL297.is_mutagen,MODEL297.is_mutagen, count(MODEL297.model_id ) FROM MODEL MODEL297  WHERE MODEL297.logp='1' group by MODEL297.is_mutagen , MODEL297.is_mutagen;
SELECT MODEL297.is_mutagen,MODEL297.lumo, count(MODEL297.model_id ) FROM MODEL MODEL297  WHERE MODEL297.logp='1' group by MODEL297.lumo , MODEL297.is_mutagen;
SELECT MODEL297.is_mutagen,MODEL297.logp, count(MODEL297.model_id ) FROM MODEL MODEL297  WHERE MODEL297.logp='1' group by MODEL297.logp , MODEL297.is_mutagen;
SELECT MODEL306.is_mutagen, count(MODEL306.model_id ) FROM MODEL MODEL306, ATOM T1008290441690  WHERE MODEL306.model_id=T1008290441690.model_id AND MODEL306.logp='0' group by MODEL306.is_mutagen;
SELECT MODEL306.is_mutagen, count(MODEL306.model_id ) FROM MODEL MODEL306, BOND T1008290441960  WHERE MODEL306.model_id=T1008290441960.model_id AND MODEL306.logp='0' group by MODEL306.is_mutagen;
SELECT MODEL306.is_mutagen,MODEL306.is_mutagen, count(MODEL306.model_id ) FROM MODEL MODEL306  WHERE MODEL306.logp='0' group by MODEL306.is_mutagen , MODEL306.is_mutagen;
SELECT MODEL306.is_mutagen,MODEL306.lumo, count(MODEL306.model_id ) FROM MODEL MODEL306  WHERE MODEL306.logp='0' group by MODEL306.lumo , MODEL306.is_mutagen;
SELECT MODEL306.is_mutagen,MODEL306.logp, count(MODEL306.model_id ) FROM MODEL MODEL306  WHERE MODEL306.logp='0' group by MODEL306.logp , MODEL306.is_mutagen;
