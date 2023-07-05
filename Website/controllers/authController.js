const { User } = require("../models");
const bcrypt = require("bcrypt");
const jwt = require("jsonwebtoken");

exports.register = async (req, res) => {
  const { username, password, device_id, key_id } = req.body;

  try {
    // Cek apakah user sudah terdaftar
    const user = await User.findOne({
      where: {
        username,
      },
    });

    if (user) {
      return res.status(400).json({
        message: "Username sudah terdaftar",
      });
    }

    // Buat hash password
    const salt = await bcrypt.genSalt(10);
    const hashedPassword = await bcrypt.hash(password, salt);

    // Simpan user baru ke database
    const newUser = await User.create({
      username,
      device_id,
      key_id,
      password: hashedPassword,
    });

    // Generate token
    const token = jwt.sign({ id: newUser.id }, process.env.JWT_SECRET);

    // Response sukses
    return res.status(200).json({
      message: "Pendaftaran berhasil",
      token,
      user: {
        id: newUser.id,
        username: newUser.username,
        device_id: newUser.device_id,
        key_id: newUser.key_id,
      },
    });
  } catch (error) {
    console.log(error);
    return res.status(500).json({
      message: "Terjadi kesalahan pada server",
    });
  }
};

exports.login = async (req, res) => {
  const { username, password } = req.body;

  try {
    // Cek apakah user terdaftar
    const user = await User.findOne({
      where: {
        username,
      },
    });

    if (!user) {
      return res.status(401).json({
        message: "Username atau password salah",
      });
    }

    // Cek password
    const isMatch = await bcrypt.compare(password, user.password);

    if (!isMatch) {
      return res.status(401).json({
        message: "Username atau password salah",
      });
    }

    // Generate token
    const token = jwt.sign({ id: user.id }, process.env.JWT_SECRET);

    // Response sukses
    return res.status(200).json({
      message: "Login berhasil",
      token,
      user: {
        id: user.id,
        username: user.username,
      },
    });
  } catch (error) {
    console.log(error);
    return res.status(500).json({
      message: "Terjadi kesalahan pada server",
    });
  }
};
