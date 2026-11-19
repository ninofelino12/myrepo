const express = require('express');
const axios = require('axios');
const path = require('path');
require('dotenv').config({ path: path.join(__dirname, 'rtdb.env') });

const app = express();
const PORT = process.env.PORT || 3000;
const RTDB_URL = process.env.RTDB_URL;
const API_KEY = process.env.API_KEY;

app.set('view engine', 'ejs');
app.set('views', path.join(__dirname, 'views'));
app.use(express.static(path.join(__dirname, 'public')));
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Helper to fetch data from Firebase RTDB REST API
async function fetchRTDB(endpoint) {
    if (!RTDB_URL) {
        throw new Error('RTDB_URL is not defined in environment variables.');
    }
    let url = `${RTDB_URL}/${endpoint}.json`;
    if (API_KEY) {
        url += `?auth=${API_KEY}`;
    }
    try {
        const response = await axios.get(url);
        return response.data;
    } catch (error) {
        try {
            const fallbackUrl = `${RTDB_URL}/${endpoint}.json`;
            const response = await axios.get(fallbackUrl);
            return response.data;
        } catch (err) {
            console.error(`Error fetching RTDB path /${endpoint}:`, err.message);
            return null;
        }
    }
}

// Routes
app.get('/', async (req, res) => {
    try {
        const bootEventsRaw = await fetchRTDB('bootEvents');
        const devicesRaw = await fetchRTDB('devices');

        let bootEvents = [];
        if (bootEventsRaw) {
            bootEvents = Object.keys(bootEventsRaw).map(key => ({
                id: key,
                ...bootEventsRaw[key]
            })).reverse();
        }

        let devices = [];
        if (devicesRaw) {
            devices = Object.keys(devicesRaw).map(key => ({
                mac: key,
                ...devicesRaw[key]
            }));
        }

        res.render('index', {
            rtdbUrl: RTDB_URL,
            bootEvents,
            devices,
            error: null,
            lastUpdated: new Date().toLocaleString()
        });
    } catch (err) {
        res.render('index', {
            rtdbUrl: RTDB_URL || 'Not configured',
            bootEvents: [],
            devices: [],
            error: err.message,
            lastUpdated: new Date().toLocaleString()
        });
    }
});

app.get('/api/logs', async (req, res) => {
    try {
        const bootEventsRaw = await fetchRTDB('bootEvents');
        const devicesRaw = await fetchRTDB('devices');

        let bootEvents = [];
        if (bootEventsRaw) {
            bootEvents = Object.keys(bootEventsRaw).map(key => ({
                id: key,
                ...bootEventsRaw[key]
            })).reverse();
        }

        let devices = [];
        if (devicesRaw) {
            devices = Object.keys(devicesRaw).map(key => ({
                mac: key,
                ...devicesRaw[key]
            }));
        }

        res.json({
            success: true,
            bootEvents,
            devices,
            lastUpdated: new Date().toISOString()
        });
    } catch (err) {
        res.status(500).json({ success: false, error: err.message });
    }
});

// Local listen if run directly
if (require.main === module) {
    app.listen(PORT, () => {
        console.log(`ESP32 RTDB Monitor Server running at http://localhost:${PORT}`);
        console.log(`Connected to RTDB: ${RTDB_URL}`);
    });
}

// Export for Vercel serverless
module.exports = app;
