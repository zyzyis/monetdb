declare function foo ($x as xs:string) as xs:string
{
    "foo"
};
declare function foo ($x as xs:integer) as xs:string
{
    "foo"
};
foo (42), foo ("bar")
