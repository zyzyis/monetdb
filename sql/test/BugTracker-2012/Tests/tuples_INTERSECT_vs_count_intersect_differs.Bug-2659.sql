CREATE TABLE T (a  INTEGER NOT NULL,
                                b  INTEGER NOT NULL,
                                c  INTEGER NOT NULL,
                                d  INTEGER NOT NULL,
                                e  INTEGER NOT NULL);


insert into T values (7,7,7,7,7);
insert into T values (82,82,82,82,82);
insert into T values (75,75,75,75,75);
insert into T values (48,48,48,48,48);
insert into T values (43,43,43,43,43);
insert into T values (87,87,87,87,87);
insert into T values (53,53,53,53,53);
insert into T values (84,84,84,84,84);
insert into T values (6,6,6,6,6);
insert into T values (96,96,96,96,96);
insert into T values (21,21,21,21,21);
insert into T values (8,8,8,8,8);
insert into T values (72,72,72,72,72);
insert into T values (39,39,39,39,39);
insert into T values (74,74,74,74,74);
insert into T values (9,9,9,9,9);
insert into T values (67,67,67,67,67);
insert into T values (73,73,73,73,73);
insert into T values (29,29,29,29,29);
insert into T values (78,78,78,78,78);
insert into T values (97,97,97,97,97);
insert into T values (58,58,58,58,58);
insert into T values (18,18,18,18,18);
insert into T values (90,90,90,90,90);
insert into T values (46,46,46,46,46);
insert into T values (79,79,79,79,79);
insert into T values (15,15,15,15,15);
insert into T values (3,3,3,3,3);
insert into T values (11,11,11,11,11);
insert into T values (28,28,28,28,28);
insert into T values (89,89,89,89,89);
insert into T values (66,66,66,66,66);
insert into T values (14,14,14,14,14);
insert into T values (57,57,57,57,57);
insert into T values (47,47,47,47,47);
insert into T values (55,55,55,55,55);
insert into T values (68,68,68,68,68);
insert into T values (41,41,41,41,41);
insert into T values (24,24,24,24,24);
insert into T values (20,20,20,20,20);
insert into T values (54,54,54,54,54);
insert into T values (50,50,50,50,50);
insert into T values (17,17,17,17,17);
insert into T values (93,93,93,93,93);
insert into T values (31,31,31,31,31);
insert into T values (5,5,5,5,5);
insert into T values (63,63,63,63,63);
insert into T values (45,45,45,45,45);
insert into T values (19,19,19,19,19);
insert into T values (10,10,10,10,10);
insert into T values (49,49,49,49,49);
insert into T values (32,32,32,32,32);
insert into T values (38,38,38,38,38);
insert into T values (34,34,34,34,34);
insert into T values (40,40,40,40,40);
insert into T values (59,59,59,59,59);
insert into T values (65,65,65,65,65);
insert into T values (4,4,4,4,4);
insert into T values (62,62,62,62,62);
insert into T values (51,51,51,51,51);
insert into T values (86,86,86,86,86);
insert into T values (94,94,94,94,94);
insert into T values (81,81,81,81,81);
insert into T values (70,70,70,70,70);
insert into T values (85,85,85,85,85);
insert into T values (88,88,88,88,88);
insert into T values (98,98,98,98,98);
insert into T values (26,26,26,26,26);
insert into T values (69,69,69,69,69);
insert into T values (35,35,35,35,35);
insert into T values (2,2,2,2,2);
insert into T values (42,42,42,42,42);
insert into T values (60,60,60,60,60);
insert into T values (80,80,80,80,80);
insert into T values (33,33,33,33,33);
insert into T values (99,99,99,99,99);
insert into T values (91,91,91,91,91);
insert into T values (13,13,13,13,13);
insert into T values (36,36,36,36,36);
insert into T values (95,95,95,95,95);
insert into T values (23,23,23,23,23);
insert into T values (77,77,77,77,77);
insert into T values (92,92,92,92,92);
insert into T values (44,44,44,44,44);
insert into T values (83,83,83,83,83);
insert into T values (76,76,76,76,76);
insert into T values (64,64,64,64,64);
insert into T values (16,16,16,16,16);
insert into T values (27,27,27,27,27);
insert into T values (52,52,52,52,52);
insert into T values (12,12,12,12,12);
insert into T values (1,1,1,1,1);
insert into T values (37,37,37,37,37);
insert into T values (100,100,100,100,100);
insert into T values (61,61,61,61,61);
insert into T values (71,71,71,71,71);
insert into T values (56,56,56,56,56);
insert into T values (22,22,22,22,22);
insert into T values (25,25,25,25,25);
insert into T values (30,30,30,30,30);
insert into T values (12,12,12,12,12);
insert into T values (1,1,1,1,1);
insert into T values (37,37,37,37,37);
insert into T values (100,100,100,100,100);
insert into T values (61,61,61,61,61);
insert into T values (71,71,71,71,71);
insert into T values (56,56,56,56,56);
insert into T values (22,22,22,22,22);
insert into T values (25,25,25,25,24);
insert into T values (30,30,30,30,10);

select * from T intersect select * from T;
select count(*) from (select * from T intersect select * from T ) as cnt;

DROP TABLE T;
