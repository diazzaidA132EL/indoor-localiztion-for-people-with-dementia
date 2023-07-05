const express = require("express");
const router = express.Router();
const locationController = require("../controllers/locationController");
const authMiddleware = require("../middlewares/authMiddleware");

// router.post("/", authMiddleware, locationController.create);
router.post("/", locationController.create);
router.get("/", authMiddleware, locationController.getAll);

module.exports = router;
