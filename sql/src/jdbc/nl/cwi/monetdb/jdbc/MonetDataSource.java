package nl.cwi.monetdb.jdbc;

import java.sql.*;
import javax.sql.*;
import java.util.*;
import java.io.*;

/**
 * A DataSource suitable for the Monet database
 * <br /><br />
 * This DataSource allows retrieval of a Connection using the JNDI bean like
 * framework.  A DataSource has numerous advantages over using the DriverManager
 * to retrieve a Connection object.  Using the DataSource interface enables a
 * more transparent application where the location or database can be changed
 * without changing any application code.
 * <br /><br />
 * Additionally, pooled connections can be used when using a DataSource.
 *
 * @author Fabian Groffen <Fabian.Groffen@cwi.nl>
 * @version 0.1
 */
public class MonetDataSource implements DataSource {
	private String databaseName;
	private String hostName;
	private int portNumber;
	private String description;
	private int loginTimeout = 0;
	private String user;
	// insecure, but no big issue as long as Monet itself doesn't do decent authorisation
	private String password;

	// the following properties are also standard:
	// private String dataSourceName;
	// private String networkProtocol;
	// private String serverName;
	// private String role;


	private MonetDriver driver;

	/**
	 * Constructor of a MonetDataSource which uses default settings for a
	 * connection.  You probably want to change these settings using the
	 * methods setDatabaseName, setServerName, setPortNumber, etc.
	 */
	public MonetDataSource() {
		databaseName = "demo";
		hostName = "localhost";
		portNumber = 45123;
		description = "MonetDB database";

		driver = new MonetDriver();
	}

	/**
	 * Attempts to establish a connection with the data source that this
	 * DataSource object represents.
	 *
	 * @return a MonetConnection
	 * @throws SQLException if connecting to the database fails
	 */
	public Connection getConnection() throws SQLException {
		return(getConnection(user, password));
	}

	/**
	 * Attempts to establish a connection with the data source that this
	 * DataSource object represents.
	 *
	 * @param username the username to use
	 * @param password the password to use
	 * @return a MonetConnection
	 * @throws SQLException if connecting to the database fails
	 */
	public Connection getConnection(String username, String password) throws SQLException {
		if (loginTimeout > 0) {
			/// could enable Socket.setSoTimeout(int timeout) here...
		}
		Properties props = new Properties();
		props.put("user", username);
		props.put("password", password);

		return(driver.connect(
			"jdbc:monetdb://" + hostName + ":" + portNumber + "/" + databaseName,
			props
		));
	}


	/**
	 * Gets the maximum time in seconds that this data source can wait while
	 * attempting to connect to a database.
	 *
	 * @return login timeout default is 0 (infinite)
	 */
	public int getLoginTimeout() {
		return(loginTimeout);
	}

	/**
	 * Sets the maximum time in seconds that this data source will wait while
	 * attempting to connect to a database.
	 *
	 * @param seconds the number of seconds to wait before aborting the connect
	 */
	public void setLoginTimeout(int seconds) {
		loginTimeout = seconds;
	}

	/**
	 * Retrieves the log writer for this DataSource object.
	 *
	 * @return null, since there is no log writer
	 */
	public PrintWriter getLogWriter() {
		return(null);
	}

	/**
	 * Sets the log writer for this DataSource object to the given
	 * java.io.PrintWriter object.
	 *
	 * @param out a PrintWriter - ignored
	 */
	public void setLogWriter(PrintWriter out) {
	}

	/**
	 * Sets the password to use when connecting.  There is no getter
	 * for obvious reasons.
	 *
	 * @param password the password
	 */
	public void setPassword(String password) {
		this.password = password;
	}

	/**
	 * Gets the username
	 *
	 * @return the username
	 */
	public String getUser() {
		return(user);
	}

	/**
	 * Sets the username
	 *
	 * @param user the username
	 */
	public void setUser(String user) {
		this.user = user;
	}

	/**
	 * Gets the database name
	 *
	 * @return the database name
	 */
	public String getDatabaseName() {
		return(databaseName);
	}

	/**
	 * Sets the database name
	 *
	 * @param user the database name
	 */
	public void setDatabaseName(String databaseName) {
		this.databaseName = databaseName;
	}

	/**
	 * Gets the description
	 *
	 * @return the description
	 */
	public String getDescription() {
		return(description);
	}

	/**
	 * Sets the description
	 *
	 * @param user the description
	 */
	public void setDescription(String description) {
		this.description = description;
	}
}