
import datetime
import types
import time
#from sets import ImmutableSet

from monetdb.sql import type_codes


# TODO:  this code is a mess. please clean me up

Date = datetime.date
Time = datetime.time
Timestamp = datetime.datetime


def string_literal(s, d):
    s = s.replace( "\\", "\\\\")
    s = s.replace( "'", "\\'")
    return "'%s'" % s

def string_deliteral(str):
    """ removes quotes from string representation """
    str = str.replace("\\\\", "\\")
    str = str.replace("\\'", "'")
    str = str.replace('\\"', '"')
    return str[1:-1]


def Str2Set(s):
    values = s.split(',')
    return apply(Set, tuple(values))

def Thing2Str(s, d):
    """Convert something into a string via str()."""
    return str(s)

def Unicode2Str(s, d):
    """Convert a unicode object to a string using utf-8 encoding."""
    return string_literal(s.encode('utf-8'), d)

# Python 1.5.2 compatibility hack
#if str(0L)[-1]=='L':
#    def Long2Int(l, d):
#        """Convert a long integer to a string, chopping the L."""
#        return str(l)[:-1]
#else:
Long2Int = Thing2Str

def None2NULL(o, d):
    """Convert None to NULL."""
    return 'NULL' # duh

def Thing2Literal(o, d):
    return string_literal(o, d)


def Instance2Str(o, d):
    """
    Convert an Instance to a string representation.  If the __str__()
    method produces acceptable output, then you don't need to add the
    class to conversions; it will be handled by the default
    converter. If the exact class is not found in d, it will use the
    first class it can find for which o is an instance.
    """

    if d.has_key(o.__class__):
        return d[o.__class__](o, d)
    cl = filter(lambda x,o=o:
                type(x) is types.ClassType
                and isinstance(o, x), d.keys())
    if not cl and hasattr(types, 'ObjectType'):
        cl = filter(lambda x,o=o:
                    type(x) is types.TypeType
                    and isinstance(o, x), d.keys())
    if not cl:
        return d[types.StringType](o,d)
    d[o.__class__] = d[cl[0]]
    return d[cl[0]](o, d)

def array2Str(o, d):
    return Thing2Literal(o.tostring(), d)

def escape_sequence(x, d):
    return Thing2Literal(str(x), d)

def escape_dict(x, d):
    return Thing2Literal(str(x), d)

def str2bool(x):
    if type(x) == types.StringType:
        return x.lower()[0]=='t'
    return bool(x)

def str2blob(x):
    """ Converts a monet blob in string representation to a python string.
    The input is a string in the format: '(length: char char char char ... )'
    w/ char in hex representation. The output is the string of chars. """
    x = x[x.find(":")+2:-1]
    return ''.join(map(lambda x: chr(int(x, 16)), x.split(" ")))

def DateFromTicks(ticks):
    return Date(*time.localtime(ticks)[:3])


def TimeFromTicks(ticks):
    return Time(*time.localtime(ticks)[3:6])


def TimestampFromTicks(ticks):
    return Timestamp(*time.localtime(ticks)[:6])

def fromTime(x):
    return apply(Time, map(lambda x: int(float(x)), x.split(":")))

def fromDate(x):
    return apply(Date, map(lambda x: int(float(x)), x.split("-")))

def fromTimestamp(x):
    x = x.split(" ")
    return apply(Timestamp,
                 map(lambda x: int(float(x)), x[0].split("-")) +
                 map(lambda x: int(float(x)), x[1].split(":"))
                 )


def Binary(x):
    return str(x)

py2mapi_map = {
    int: Thing2Str,
    float: Thing2Str,
    type(None): None2NULL,
    tuple: escape_sequence,
    list: escape_sequence,
    dict: escape_dict,
    #??
    #__builtins__['instance']: Instance2Str,
    str: Thing2Literal,
}

if hasattr(__builtins__, 'long'):
    # python 3 doesn't have long
    py2mapi_map[long] = Long2Int

conversions = {
    # Python -> Monet
    #types.IntType: Thing2Str,
    #types.LongType: Long2Int,
    #types.FloatType: Thing2Str,
    #types.NoneType: None2NULL,
    #types.TupleType: escape_sequence,
    #types.ListType: escape_sequence,
    #types.DictType: escape_dict,
    #types.InstanceType: Instance2Str,
    # array.ArrayType: array2Str,
    # FIXME: date/time types
    #types.StringType: Thing2Literal, # default

    # Monet SQL -> Python
    "varchar": string_deliteral,
    "tinyint": int,
    "smallint": int,
    "shortint": int,
    "mediumint": int,
    "bigint": int, #long,
    "longint": int, #long,
    "boolean": str2bool,
    "decimal": float,
    "float": float,
    "double": float,
    "real": float,
    "blob": str2blob,
    # timestamp types
    "date": fromDate,
    "time": fromTime,
    "timestamp": fromTimestamp,

    # Monet MIL -> Python
    "str": str,
    "bte": int,
    "sht": int,
    "int": int,
    "lng": int, #long,
    "wrd": int, #long,
    "bit": bool,
    "chr": str,
    "flt": float,
    "dbl": float,
    "oid": str
    }


if hasattr(types, 'UnicodeType'):
    conversions[types.UnicodeType] = Unicode2Str

if hasattr(types, 'ObjectType'):
    conversions[types.ObjectType] = Instance2Str

def escape(x, conv=None):
    """ Escapes """

    if conv is None:
        conv = py2mapi_map
        #conv = conversions

    if type(x) is types.ListType or type(x) is types.TupleType:
        y = []
        for a in x:
            if conv.has_key(type(a)):
                y.append(conv[type(a)](a, conv))
        return tuple(y)
    if type(x) is types.DictType:
        for a in x:
            #if conv.has_key(type(x[a])):
            if type(x[a]) in conv:
                x[a] = conv[type(x[a])](x[a], conv)
        return x

    # single value
    if conv.has_key(type(x)):
        x = conv[type(x)](x, conv)
    return x


def monet2python(value, typestr, conv=conversions):
    """ convert mapi tuple values to python types """

    # SQL NULL values are represented by the Python None singleton on input and output.
    if value == "NULL":
        return None

    if typestr in conv:
        return conv[typestr](value)
    return value







#class DBAPISet(ImmutableSet):
class DBAPISet(frozenset):
    """A special type of set for which A == x is true if A is a
    DBAPISet and x is a member of that set."""

    def __ne__(self, other):
        from sets import BaseSet
        if isinstance(other, BaseSet):
            return super(DBAPISet.self).__ne__(self, other)
        else:
            return other not in self

    def __eq__(self, other):
        #from sets import BaseSet
        #if isinstance(other, BaseSet):
        if isinstance(other, set):
            return super(DBAPISet, self).__eq__(self, other)
        else:
            return other in self


STRING = DBAPISet([type_codes.VARCHAR])
BINARY    = DBAPISet([type_codes.BLOB])
NUMBER    = DBAPISet([type_codes.DECIMAL, type_codes.DOUBLE, type_codes.REAL, type_codes.BIGINT, type_codes.SMALLINT])
DATE      = DBAPISet([type_codes.DATE])
TIME      = DBAPISet([type_codes.TIME])
TIMESTAMP = DBAPISet([type_codes.TIMESTAMP])
DATETIME  = TIMESTAMP
ROWID     = DBAPISet()


