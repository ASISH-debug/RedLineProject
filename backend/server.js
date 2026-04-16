const express = require("express");
const cors = require("cors");
const bodyParser = require("body-parser");
const app = express();

app.use(cors());
app.use(bodyParser.json());

// Temporary in-memory data
let bookings = [];

// Home route
app.get("/", (req, res) => {
  res.send("🚍 Redline Auto Booking Backend is running (No Database)");
});

// Get all bookings
app.get("/api/bookings", (req, res) => {
  res.json(bookings);
});

// Add a new booking
app.post("/api/bookings", (req, res) => {
  const newBooking = { id: bookings.length + 1, ...req.body };
  bookings.push(newBooking);
  res.json({ success: true, message: "✅ Booking added successfully", booking: newBooking });
});

// Clear all bookings (optional, for testing)
app.delete("/api/bookings", (req, res) => {
  bookings = [];
  res.json({ message: "🧹 All bookings cleared" });
});

const PORT = 5050;
app.listen(PORT, () => console.log(`✅ Server started on port ${PORT}`));