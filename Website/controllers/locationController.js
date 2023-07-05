const models = require("../models/index");
const Location = models.Location;

const MAXIMUM_DATA_ROWS = 1440;

exports.create = async (req, res) => {
  const { deviceName, latitude, longitude, location, activity } = req.body;

  try {
    // Save location to database
    const newLocation = await Location.create({
      deviceName,
      latitude,
      longitude,
      location,
      activity,
    });

    // Check if number of rows exceeds the limit
    const rowCount = await Location.count();
    if (rowCount > MAXIMUM_DATA_ROWS) {
      // Find the oldest data and delete it
      const oldestLocation = await Location.findOne({
        order: [["createdAt", "ASC"]],
      });
      await oldestLocation.destroy();
    }

    // Response success
    return res.status(200).json({
      message: "Location saved successfully",
      location: newLocation,
    });
  } catch (error) {
    console.log(error);
    return res.status(500).json({
      message: "Internal server error",
    });
  }
};

exports.getAll = async (req, res) => {
  const { page = 1, limit = 10 } = req.query; // Ambil query parameter page dan limit, default nilai adalah 1 dan 10

  try {
    const totalCount = await Location.count(); // Menghitung total jumlah data

    const locations = await Location.findAll({
      limit: +limit, // Mengubah nilai limit menjadi tipe data numerik
      offset: (page - 1) * +limit, // Menghitung offset berdasarkan halaman dan limit
    });

    return res.status(200).json({
      message: "Locations retrieved successfully",
      locations,
      totalPages: Math.ceil(totalCount / +limit), // Menghitung jumlah total halaman
      currentPage: +page, // Mengirimkan informasi halaman saat ini
    });
  } catch (error) {
    console.log(error);
    return res.status(500).json({
      message: "Internal server error",
    });
  }
};
