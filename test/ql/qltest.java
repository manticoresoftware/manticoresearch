import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.logging.Level;
import java.util.logging.Logger;

public class qltest
{
	public static void main(String[] args) throws SQLException
	{
		Connection con = null;
		Statement st = null;
		ResultSet rs = null;

		String url = "jdbc:mysql://localhost:9306/testdb";
		String user = "root";
		String password = "";

		con = DriverManager.getConnection(url, user, password);
		st = con.createStatement();
		rs = st.executeQuery("SELECT * FROM rt");

		while (rs.next()) {
			System.out.println(
				rs.getString(1) + " | " +
				rs.getString(2) + " | " +
				rs.getString(3) + " | " +
				rs.getString(4));
		}
	}
}
