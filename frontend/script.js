console.log("SCRIPT.JS >>> loaded");

document.addEventListener("DOMContentLoaded", () => {
  const form = document.getElementById("bookingForm");
  const resultsDiv = document.getElementById("results");
  const API_BASE = "http://127.0.0.1:5050/api";

  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    console.log("Submitting search:", {
      source: document.getElementById("source").value,
      destination: document.getElementById("destination").value,
      date: document.getElementById("date").value
    });

    const data = {
      source: document.getElementById("source").value,
      destination: document.getElementById("destination").value,
      travelDate: document.getElementById("date").value,
    };
    if (!data.source || !data.destination || !data.travelDate) {
      alert("Please fill all fields");
      return;
    }
    resultsDiv.innerHTML = `<p style="text-align:center;">🔎 Searching buses...</p>`;
    try {
      const res = await fetch(`${API_BASE}/buses`);
      const buses = await res.json();
      resultsDiv.innerHTML = `
        <h2 style="text-align:center;">🚌 Available Buses</h2>
        ${buses.map((bus, index) => `
          <div class="bus-card">
            <h3>${bus.name}</h3>
            <p>Fare: ₹${bus.fare} • Seats: ${bus.seatCount}</p>
            <button data-id="${bus.id}" class="book-now">Book Now</button>
          </div>
        `).join("")}
      `;
      document.querySelectorAll(".book-now").forEach(btn=>{
        btn.addEventListener("click", ()=> openSeatSelection(Number(btn.dataset.id), data));
      });
    } catch (err) {
      console.error(err);
      alert("Could not fetch buses. Is the backend running on port 5050?");
    }
  });

  async function openSeatSelection(busId, searchData) {
    try {
      const res = await fetch(`${API_BASE}/buses/${busId}/seats`);
      if (!res.ok) { alert("Could not load seats"); return; }
      const seats = await res.json();
      const seatCount = seats.length;
      const perRow = 5;
      resultsDiv.innerHTML = `
        <h2 style="text-align:center;">Select Seats — Bus ${busId}</h2>
        <div id="seatGrid" class="seat-grid">${renderSeatGridHtml(seats, perRow)}</div>
        <div style="max-width:420px;margin:12px auto;">
          <form id="paymentForm">
            <label>Your name:</label>
            <input type="text" id="passengerName" required />
            <label>Selected seats: <span id="selectedSeatsLabel">None</span></label>
            <label>Fare per seat: <span id="farePerSeat">-</span></label>
            <p style="font-size:13px;color:#666;">(Dummy payment fields below)</p>
            <label>Card Number:</label><input type="text" id="cardNumber" maxlength="16" required />
            <label>Expiry:</label><input type="month" id="expiry" required />
            <label>CVV:</label><input type="password" id="cvv" maxlength="3" required />
            <button type="submit">Pay & Confirm</button>
          </form>
        </div>
      `;
      const busesRes = await fetch(`${API_BASE}/buses`);
      const buses = await busesRes.json();
      const bus = buses.find(b=>b.id===busId);
      document.getElementById("farePerSeat").innerText = bus ? `₹${bus.fare}` : "-";
      const selected = new Set();
      document.querySelectorAll(".seat-item").forEach(el=>{
        if (el.dataset.booked === "true") return;
        el.addEventListener("click", ()=> {
          const num = Number(el.dataset.number);
          if (selected.has(num)) {
            selected.delete(num); el.classList.remove("seat-selected");
          } else {
            selected.add(num); el.classList.add("seat-selected");
          }
          document.getElementById("selectedSeatsLabel").innerText = selected.size ? Array.from(selected).join(", ") : "None";
        });
      });

      const paymentForm = document.getElementById("paymentForm");
      paymentForm.addEventListener("submit", async (e) => {
        e.preventDefault();
        if (selected.size === 0) { alert("Select at least one seat"); return; }
        const bookingPayload = {
          busId: busId,
          seatNumbers: Array.from(selected),
          name: document.getElementById("passengerName").value,
          fare: bus ? bus.fare * selected.size : 0,
          source: searchData.source,
          destination: searchData.destination,
          travelDate: searchData.travelDate
        };
        try {
          const r = await fetch(`${API_BASE}/bookings`, {
            method: "POST",
            headers: {"Content-Type":"application/json"},
            body: JSON.stringify(bookingPayload)
          });
          if (r.status === 200 || r.status === 201) {
            const json = await r.json();
            showSuccess(json.booking);
          } else {
            const err = await r.json().catch(()=>({message: "Unknown error"}));
            alert("Booking failed: " + (err.error || JSON.stringify(err)));
            openSeatSelection(busId, searchData);
          }
        } catch (err) {
          console.error(err);
          alert("Could not reach backend. Check server and CORS.");
        }
      });

    } catch (err) {
      console.error(err);
      alert("Error loading seats: " + err.message);
    }
  }

  function renderSeatGridHtml(seats, perRow) {
    let html = '<div class="seat-grid-wrap">';
    seats.forEach((s, idx) => {
      const cls = s.booked ? "seat-item seat-booked" : "seat-item seat-free";
      html += `<div class="${cls}" data-number="${s.number}" data-booked="${s.booked}">${s.number}</div>`;
      if ((idx+1) % perRow === 0) html += '<div class="clear"></div>';
    });
    html += '</div>';
    return html;
  }

  function showSuccess(booking) {
    resultsDiv.innerHTML = `
      <div class="success-card">
        <h2>🎉 Booking Confirmed!</h2>
        <p>Bus: <b>${booking.busName}</b></p>
        <p>Seats: <b>${booking.seatNumbers.join(", ")}</b></p>
        <p>Date: <b>${booking.travelDate || "-"}</b></p>
        <p>Passenger: <b>${booking.name}</b></p>
        <div class="ticket-number">Ticket#${booking.id}</div>
        <button id="viewBookings" style="margin-top:12px;">View All Bookings</button>
      </div>
    `;
    document.getElementById("viewBookings").addEventListener("click", showBookingsFromBackend);
  }

  async function showBookingsFromBackend() {
    try {
      const res = await fetch(`${API_BASE}/bookings`);
      if (!res.ok) { alert("No bookings or server error: " + res.status); return; }
      const bookings = await res.json();
      resultsDiv.innerHTML = `<h2 style="text-align:center;">📚 Saved Bookings</h2>
        ${bookings.map(b => `
          <div class="bus-card">
            <strong>Ticket#${b.id}</strong>
            <div>${b.busName} • ${b.source} → ${b.destination}</div>
            <div>Date: ${b.travelDate || ""} • Fare: ₹${b.fare || ""}</div>
            <div>Seats: ${b.seatNumbers.join(", ")}</div>
          </div>
        `).join("")}
      `;
    } catch (err) {
      console.error(err);
      alert("Could not fetch bookings. Check backend or CORS.");
    }
  }

});