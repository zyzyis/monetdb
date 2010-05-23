create schema intro1;
create schema intro2 authorized monetdb;
create schema authorized monetdb;

drop schema intro1;
drop schema intro2;

create schema intro1
 default character set whatever;

-- following gives unexpected error
create table t2010(i int);
create schema intro3 
  grant insert on t2010 to monetdb;

