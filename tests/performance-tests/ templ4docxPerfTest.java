// Compile with: (example)
//   javac -cp "libs/*" "tests/performance-tests/templ4docxPerfTest.java"
// Run with:
//   java  -cp "libs/*:tests/performance-tests" templ4docxPerfTest

import pl.jsolve.templ4docx.core.Docx;
import pl.jsolve.templ4docx.variable.TableVariable;
import pl.jsolve.templ4docx.variable.TextVariable;
import pl.jsolve.templ4docx.variable.Variable;
import pl.jsolve.templ4docx.variable.Variables;

import java.io.FileInputStream;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

// Mongo Java Driver (ensure driver jar present):
import com.mongodb.client.MongoClients;
import com.mongodb.client.MongoClient;
import com.mongodb.client.MongoDatabase;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.FindIterable;
import org.bson.Document;
import static com.mongodb.client.model.Sorts.ascending;

public class templ4docxPerfTest {

	// Adjust collection name if different
	private static final String COLLECTION = "data";
	private static final int MAX_ROWS = 2000; // cap for performance test
	private static final DateTimeFormatter MOMENT_FMT = DateTimeFormatter.ofPattern("dd-MM-yyyy HH:mm:ss");

	public static void main(String[] args) throws Exception {
		String templatePath = "tests/performance-tests/testdoc.docx"; // relative path
		String outputPath   = "tests/performance-tests/testdoc-filled.docx";

		long t0 = System.nanoTime();
		List<Row> rows = fetchData();
		long tFetch = System.nanoTime();

		Variables vars = buildVariables(rows);
		long tVars = System.nanoTime();

		try (InputStream is = new FileInputStream(templatePath)) {
			Docx docx = new Docx(is);
			// Default pattern is ${ } – ensure template uses these placeholders.
			docx.fillTemplate(vars);
			docx.save(outputPath);
		}
		long tFill = System.nanoTime();

		System.out.println("templ4docxPerfTest: rows=" + rows.size());
		System.out.printf("Fetch ms=%.2f, Vars ms=%.2f, Fill+Save ms=%.2f, Total ms=%.2f%n",
				(tFetch - t0)/1e6, (tVars - tFetch)/1e6, (tFill - tVars)/1e6, (tFill - t0)/1e6);
		System.out.println("Output: " + outputPath);
	}

	private static List<Row> fetchData() {
		List<Row> out = new ArrayList<>(1024);
		// Default local connection (no auth) – read only
		try (MongoClient client = MongoClients.create()) {
			MongoDatabase db = client.getDatabase("test"); // change if DB name differs
			MongoCollection<Document> col = db.getCollection(COLLECTION);
			FindIterable<Document> docs = col.find().sort(ascending("moment")).limit(MAX_ROWS);
			for (Document d : docs) {
				Row r = new Row();
				r.deviceId = str(d.get("deviceId"));
				r.cpm      = str(d.get("cpm"));
				r.mSvh     = str(d.get("uSvh"));
				r.mRh      = str(d.get("mRh"));
				r.mrhLevel = str(d.get("mrhLevel"));
				r.alarm    = str(d.get("alarm"));
				Object moment = d.get("moment");
				if (moment instanceof java.util.Date) {
					r.moment = ((java.util.Date)moment).toInstant().atZone(ZoneId.systemDefault()).toLocalDateTime().format(MOMENT_FMT);
				} else {
					r.moment = str(moment);
				}
				out.add(r);
			}
		}
		return out;
	}

	private static String str(Object o) { return o == null ? "" : o.toString(); }

	private static Variables buildVariables(List<Row> rows) {
		TableVariable table = new TableVariable();
		List<Variable> deviceIdCol = new ArrayList<>();
		List<Variable> cpmCol      = new ArrayList<>();
		List<Variable> mRhCol      = new ArrayList<>();
		List<Variable> mSvhCol     = new ArrayList<>();
		List<Variable> mrhLevelCol = new ArrayList<>();
		List<Variable> alarmCol    = new ArrayList<>();
		List<Variable> momentCol   = new ArrayList<>();
		for (Row r : rows) {
			deviceIdCol.add(new TextVariable("${deviceId}", r.deviceId));
			cpmCol.add(new TextVariable("${cpm}", r.cpm));
			mSvhCol.add(new TextVariable("${mSvh}", r.mSvh));
			mRhCol.add(new TextVariable("${mRh}", r.mRh));
			mrhLevelCol.add(new TextVariable("${mrhLevel}", r.mrhLevel));
			alarmCol.add(new TextVariable("${alarm}", r.alarm));
			momentCol.add(new TextVariable("${moment}", r.moment));
		}
		table.addVariable(deviceIdCol);
		table.addVariable(cpmCol);
		table.addVariable(mRhCol);
		table.addVariable(mSvhCol);
		table.addVariable(mrhLevelCol);
		table.addVariable(alarmCol);
		table.addVariable(momentCol);
		Variables vars = new Variables();
		vars.addTableVariable(table);
		return vars;
	}

	private static class Row {
		String deviceId, cpm, mSvh, mRh, mrhLevel, alarm, moment;
	}
}
