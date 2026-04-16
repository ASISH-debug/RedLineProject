const express = require("express");
const router = express.Router();

// Temporary in-memory array for bookings
let bookings = [];

// ✅ Add a new booking
router.post("/bookings", (req, res) => {
  console.log("📩 Received booking data:", req.body);

  const { userId, source, destination, travelDate, travelTime, busType, fare, requireOTP } = req.body;

  if (!userId || !source || !destination || !travelDate || !travelTime) {
    return res.status(400).json({ error: "Missing required fields" });
  }

  const newBooking = {
    id: bookings.length + 1,
    userId,
    source,
    destination,
    travelDate,
    travelTime,
    busType: busType || "Standard",
    fare: fare || 500,
    requireOTP: requireOTP || false,
    created_at: new Date().toISOString()
  };

  bookings.push(newBooking);
  console.log("✅ Booking saved:", newBooking);

  res.json({
    message: "Booking successfully added (No Database used)",
    booking: newBooking
  });
});

// ✅ Get all bookings
router.get("/bookings", (req, res) => {
  res.json(bookings);
});

// ✅ Clear all bookings (optional, for testing)
router.delete("/bookings", (req, res) => {
  bookings = [];
  res.json({ message: "🧹 All bookings cleared" });
});

module.exports = router;