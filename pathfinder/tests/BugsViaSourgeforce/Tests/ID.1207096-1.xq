for $x in doc("ID.1207096.book.xml")/bib/book
where $x/price < 100
return $x/price
