COPY 150000 RECORDS INTO customer from 'PWD/customer.tbl' USING DELIMITERS '|', '|\n';
COPY 25 RECORDS INTO nation from 'PWD/nation.tbl' USING DELIMITERS '|', '|\n';
COPY 1500000 RECORDS INTO orders from 'PWD/orders.tbl' USING DELIMITERS '|', '|\n';
COPY 800000 RECORDS INTO partsupp from 'PWD/partsupp.tbl' USING DELIMITERS '|', '|\n';
COPY 200000 RECORDS INTO part from 'PWD/part.tbl' USING DELIMITERS '|', '|\n';
COPY 5 RECORDS INTO region from 'PWD/region.tbl' USING DELIMITERS '|', '|\n';
COPY 10000 RECORDS INTO supplier from 'PWD/supplier.tbl' USING DELIMITERS '|', '|\n';
COPY 7000000 RECORDS INTO lineitem from 'PWD/lineitem.tbl' USING DELIMITERS '|', '|\n';
