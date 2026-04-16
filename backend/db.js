const mysql = require("mysql2");

const db = mysql.createConnection({
  host: "localhost",
  user: "root",       // 👈 replace with your MySQL username
  password: "your_password_here",  // 👈 replace with your password
  database: "redline_db",          // 👈 your DB name
});

db.connect((err) => {
  if (err) {
    console.error("❌ Database connection failed:", err);
  } else {
    console.log("✅ Connected to MySQL database!");
  }
});

module.exports = db;