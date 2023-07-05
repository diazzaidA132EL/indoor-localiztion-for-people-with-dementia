const express = require("express");
const app = express();
const cors = require("cors");
const bodyParser = require("body-parser");

// load environment variables
require("dotenv").config();

// setup database connection
const db = require("./config/database");

// middleware
app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

// routes
const authRoutes = require("./routes/authRoutes");
app.use("/api/auth", authRoutes);

const userRoutes = require("./routes/userRoutes");
app.use("/api/users", userRoutes);

const locationRoutes = require("./routes/locationRoutes");
app.use("/api/locations", locationRoutes);

// serve your css as static
app.use(express.static(__dirname));

app.get("/login", (req, res) => {
  res.sendFile(__dirname + "/public/login.html");
});

app.get("/index", (req, res) => {
  res.sendFile(__dirname + "/public/index.html");
});

app.get("/register", (req, res) => {
  res.sendFile(__dirname + "/public/register.html");
});

app.get("/index.js", (req, res) => {
  res.sendFile(__dirname + "/public/index.js");
});

app.get("/app.js", (req, res) => {
  res.sendFile(__dirname + "/public/app.js");
});

app.get("/assets/img/user.jpeg", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/user.jpeg");
});

app.get("/assets/img/act.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/act.png");
});

app.get("/assets/img/orang.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/orang.png");
});

app.get("/assets/img/orang2.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/orang2.png");
});

app.get("/assets/img/falling.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/falling.png");
});

app.get("/assets/img/battery.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/battery.png");
});

app.get("/assets/img/battery1.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/battery1.png");
});

app.get("/assets/img/battery2.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/battery2.png");
});

app.get("/assets/img/battery3.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/battery3.png");
});

app.get("/assets/img/battery4.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/battery4.png");
});

app.get("/assets/img/lokasi.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/lokasi.png");
});

app.get("/assets/img/peta.jpeg", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/peta.jpeg");
});

app.get("/assets/img/sleeping.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/sleeping.png");
});

app.get("/assets/img/walking.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/walking.png");
});

app.get("/assets/img/edit.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/edit.png");
});

app.get("/assets/img/history.png", (req, res) => {
  res.sendFile(__dirname + "/public/assets/img/history.png");
});

app.get("/", (req, res) => {
  res.sendFile(__dirname + "/public/index.html");
});

app.get("/env", (req, res) => {
  res.sendFile(__dirname + "/public/env.json");
});

app.get("/css1", (req, res) => {
  res.sendFile(__dirname + "/public/style.css");
});

app.get("/css2", (req, res) => {
  res.sendFile(__dirname + "/public/style-2.css");
});

app.get("/css3", (req, res) => {
  res.sendFile(__dirname + "/public/style-3.css");
});

app.get("/css4", (req, res) => {
  res.sendFile(__dirname + "/public/style-4.css");
});

app.get("/css5", (req, res) => {
  res.sendFile(__dirname + "/public/style-5.css");
});

app.get("/lokasi", (req, res) => {
  res.sendFile(__dirname + "/public/lokasi.html");
});

app.get("/history", (req, res) => {
  res.sendFile(__dirname + "/public/history.html");
});

app.get("/aktivitas", (req, res) => {
  res.sendFile(__dirname + "/public/aktivitas.html");
});

app.get("/input", (req, res) => {
  res.sendFile(__dirname + "/public/input.html");
});

// Cron Job {{ Location Log }}
const cron = require("node-cron");
const axios = require("axios");

const send_location_data = async () => {
  let api = "http://127.0.0.1:3000";
  try {
    const response = await axios.post(api + "/api/locations", {
      deviceName: "Device A",
      location: "Kamar Tidur",
      activity: "Tidur",
      latitude: 8.7,
      longitude: 2.5,
    });
    console.log(response.data);
  } catch (error) {
    console.error(error);
  }
};
// cron.schedule("* * * * * *", () => {
//   send_location_data();
// });

// start server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
