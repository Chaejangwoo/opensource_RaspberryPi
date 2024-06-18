var express = require('express');
var exec = require('child_process').exec;
var router = express.Router();

/* GET home page. */
router.get('/', function (req, res, next) {
    res.render('index', { title: 'Express' });
});

router.get('/run1', function (req, res, next) {
    exec('/home/pi/smartfarm/rgbtest', function (err, stdout, stderr) {
        if (err) {
            console.error('Error executing command:', err);
            return res.status(500).json({ error: 'Error executing command' });
        }

        console.log('Command output:', stdout);
        res.json({ output: stdout });
    });
});
router.get('/run2', function (req, res, next) {
    exec('/home/pi/smartfarm/pumpon', function (err, stdout, stderr) {
        if (err) {
            console.error('Error executing command:', err);
            return res.status(500).json({ error: 'Error executing command' });
        }

        console.log('Command output:', stdout);
        res.json({ output: stdout });
    });
});
router.get('/run3', function (req, res, next) {
    exec('/home/pi/smartfarm/fanon', function (err, stdout, stderr) {
        if (err) {
            console.error('Error executing command:', err);
            return res.status(500).json({ error: 'Error executing command' });
        }

        console.log('Command output:', stdout);
        res.json({ output: stdout });
    });
});

module.exports = router;
