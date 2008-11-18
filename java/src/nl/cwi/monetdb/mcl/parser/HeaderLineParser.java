/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the MonetDB Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 * Copyright August 2008- MonetDB B.V.
 * All Rights Reserved.
 */

package nl.cwi.monetdb.mcl.parser;

import java.util.*;
import nl.cwi.monetdb.mcl.*;

/**
 * The HeaderLineParser is a generic MCLParser that extracts values from
 * a metadata header in the MCL protocol either as string or integer
 * values.
 *
 * @author Fabian Groffen <Fabian.Groffen>
 */
public class HeaderLineParser extends MCLParser {
	private int type;

	public final static int NAME       = 1;
	public final static int LENGTH     = 2;
	public final static int TABLE      = 3;
	public final static int TYPE       = 4;

	/**
	 * Constructs a HeaderLineParser which expects columncount columns.
	 *
	 * @param columncount the number of columns in the to be parsed string
	 */
	public HeaderLineParser(int columncount) {
		super(columncount);
	}

	/**
	 * Parses the given String source as header line.  If source cannot
	 * be parsed, an MCLParseException is thrown.  The columncount argument
	 * given during construction is used for allocation of the backing
	 * array.  Parsing a header line with has more fields will therefore
	 * result in a crash.  While this seems illogical, the caller should
	 * know this size, since the StartOfHeader contains this
	 * information.
	 *
	 * @param source a String which should be parsed
	 * @return the type of then parsed header line
	 * @throws MCLParseException if an error occurs during parsing
	 */
	public int parse(String source) throws MCLParseException {
		char[] chrLine = source.toCharArray();
		int len = chrLine.length;
		int pos = 0;
		boolean foundChar = false;
		boolean nameFound = false;
		// find header name
		for (int i = len - 1; i >= 0; i--) {
			switch (chrLine[i]) {
				case ' ':
				case '\n':
				case '\t':
				case '\r':
					if (!foundChar) {
						len = i - 1;
					} else {
						pos = i + 1;
					}
					break;
				case '#':
					// found!
					nameFound = true;
					if (pos == 0) pos = i + 1;
					i = 0;	// force the loop to terminate
					break;
				default:
					foundChar = true;
					pos = 0;
					break;
			}
		}
		if (!nameFound)
			throw new MCLParseException("invalid header, no header name found", pos);

		// depending on the name of the header, we continue
		switch (chrLine[pos]) {
			case 'n':
				if (len - pos == 4 &&
						source.regionMatches(pos + 1, "name", 1, 3))
				{
					getValues(chrLine, 2, pos - 3);
					type = NAME;
					break;
				}
			case 'l':
				if (len - pos == 6 &&
						source.regionMatches(pos + 1, "length", 1, 5))
				{
					getIntValues(chrLine, 2, pos - 3);
					type = LENGTH;
					break;
				}
			case 't':
				if (len - pos == 4 &&
						source.regionMatches(pos + 1, "type", 1, 3))
				{
					getValues(chrLine, 2, pos - 3);
					type = TYPE;
					break;
				} else if (len - pos == 10 &&
						source.regionMatches(pos + 1, "table_name", 1, 9))
				{
					getValues(chrLine, 2, pos - 3);
					type = TABLE;
					break;
				}
			default:
				throw new MCLParseException("unknown header: " +
						(new String(chrLine, pos, len - pos)));
		}

		// adjust colno
		reset();

		return(type);
	}

	/**
	 * Returns an array of Strings containing the values between
	 * ',\t' separators.  Note that no quoting/dequoting is done in this
	 * method.
	 *
	 * @param chrLine a character array holding the input data
	 * @param start where the relevant data starts
	 * @param stop where the relevant data stops
	 */
	final private void getValues(char[] chrLine, int start, int stop) {
		int elem = 0;

		for (int i = start + 1; i < stop; i++) {
			if (chrLine[i] == '\t' && chrLine[i - 1] == ',') {
				values[elem++] =
					new String(chrLine, start, i - 1 - start);
				start = i + 1;
			}
		}
		// add the left over part
		values[elem++] = new String(chrLine, start, stop - start);
	}

	/**
	 * Returns an array of ints containing the values between
	 * ',\t' separators.
	 *
	 * @param chrLine a character array holding the input data
	 * @param start where the relevant data starts
	 * @param stop where the relevant data stops
	 */
	final private void getIntValues(char[] chrLine, int start, int stop)
		throws MCLParseException
	{
		int elem = 0;
		int tmp = 0;

		for (int i = start; i < stop; i++) {
			if (chrLine[i] == ',' && chrLine[i + 1] == '\t') {
				intValues[elem++] = tmp;
				tmp = 0;
				start = ++i;
			} else {
				tmp *= 10;
				// note: don't use Character.isDigit() here, because
				// we only want ISO-LATIN-1 digits
				if (chrLine[i] >= '0' && chrLine[i] <= '9') {
					tmp += (int)chrLine[i] - (int)'0';
				} else {
					throw new MCLParseException("expected a digit in " + new String(chrLine) + " at " + i);
				}
			}
		}
		// add the left over part
		intValues[elem++] = tmp;
	}
}
