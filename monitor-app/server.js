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

// Helper to parse timestamp from Firebase Push ID or object properties
function parseTimestamp(item) {
    if (item.date && item.date !== 'unknown') {
        const parsed = Date.parse(item.date);
        if (!isNaN(parsed)) return parsed;
    }
    if (item.timestamp) return Number(item.timestamp);
    if (item.createdAt) return Number(item.createdAt);
    if (item.time) return Number(item.time);
    if (item.id) {
        const pushId = item.id;
        if (pushId.length >= 8) {
            const PUSH_CHARS = '-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz';
            let timestamp = 0;
            for (let i = 0; i < 8; i++) {
                const c = pushId.charAt(i);
                const index = PUSH_CHARS.indexOf(c);
                if (index !== -1) {
                    timestamp = timestamp * 64 + index;
                }
            }
            if (timestamp > 0) return timestamp;
        }
    }
    return Date.now();
}

// Helper to aggregate data for calendar (device push count & log count per day)
function getCalendarData(bootEvents, devices) {
    const calendarMap = {};
    const registeredMacs = new Set(devices.map(d => (d.mac || "").toUpperCase().replace(/[:_]/g, '')));

    bootEvents.forEach(evt => {
        const ts = parseTimestamp(evt);
        const date = new Date(ts);
        const dateStr = date.toISOString().split('T')[0]; // YYYY-MM-DD

        if (!calendarMap[dateStr]) {
            calendarMap[dateStr] = {
                date: dateStr,
                logCount: 0,
                deviceSet: new Set(),
                logs: []
            };
        }
        calendarMap[dateStr].logCount++;
        calendarMap[dateStr].logs.push(evt);
        
        const mac = (evt.mac || "").toUpperCase().replace(/[:_]/g, '');
        if (mac && registeredMacs.has(mac)) {
            calendarMap[dateStr].deviceSet.add(mac);
        }
    });

    devices.forEach(dev => {
        const ts = parseTimestamp(dev);
        const date = new Date(ts);
        const dateStr = date.toISOString().split('T')[0];

        if (!calendarMap[dateStr]) {
            calendarMap[dateStr] = {
                date: dateStr,
                logCount: 0,
                deviceSet: new Set(),
                logs: []
            };
        }
        const mac = (dev.mac || "").toUpperCase().replace(/[:_]/g, '');
        if (mac && registeredMacs.has(mac)) {
            calendarMap[dateStr].deviceSet.add(mac);
        }
    });

    const result = {};
    Object.keys(calendarMap).forEach(dateStr => {
        result[dateStr] = {
            date: dateStr,
            logCount: calendarMap[dateStr].logCount,
            deviceCount: calendarMap[dateStr].deviceSet.size,
            devices: Array.from(calendarMap[dateStr].deviceSet),
            logs: calendarMap[dateStr].logs
        };
    });

    return result;
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

app.get('/calendar', async (req, res) => {
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

        const calendarData = getCalendarData(bootEvents, devices);

        res.render('calendar', {
            rtdbUrl: RTDB_URL,
            calendarData,
            error: null,
            lastUpdated: new Date().toLocaleString()
        });
    } catch (err) {
        res.render('calendar', {
            rtdbUrl: RTDB_URL || 'Not configured',
            calendarData: {},
            error: err.message,
            lastUpdated: new Date().toLocaleString()
        });
    }
});

app.get('/logs', async (req, res) => {
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

        res.render('logs', {
            rtdbUrl: RTDB_URL,
            bootEvents,
            devices,
            error: null,
            lastUpdated: new Date().toLocaleString()
        });
    } catch (err) {
        res.render('logs', {
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
