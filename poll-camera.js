#!/usr/bin/nodejs

'use strict';

const http = require('http');

const cameras = [
  {
    url: 'http://sidewalkcam1.local/readings',
    reading: false,
    waiting: false
  },
  {
    url: 'http://sidewalkcam2.local/readings',
    reading: false,
    waiting: false
  }
];

let combined_reading = false;
let waiting = false;

const onPoll = () => {
  const reading = cameras[0].reading && cameras[1].reading;
  // if (combined_reading != reading) {
    combined_reading = reading;
    console.log(combined_reading ? '1' : '0');
  // }
};

const pollCamera = (camera, i) => {
  if (!camera.waiting) {
    camera.waiting = true;
    http.get(camera.url, (res) => {
      const status_code = res.statusCode;
      const content_type = res.headers['content-type'];

      let error;
      if (status_code !== 200) {
        error = new Error(`Request Failed. (Status Code: ${status_code})`);
      }
      else if (!/^application\/json/.test(content_type)) {
        error = new Error(`Invalid content-type. (Expected application/json but received ${contentType})`);
      }
      if (error) {
        console.error(error.message);
        // consume response data to free up memory
        res.resume();
        camera.waiting = false;
        return;
      }

      res.setEncoding('utf8');
      let data = '';
      res.on('data', (chunk) => { data += chunk; });
      res.on('end', () => {
        try {
          const json = JSON.parse(data);
          camera.reading = !!json.a1;

          // console.log(`camera ${i}: ${camera.reading}`);

          onPoll();
        }
        catch (err) {
          console.error(err.message);
        }
        camera.waiting = false;
      });
    }).on('error', (err) => {
      camera.waiting = false;
    });
  }
};

setInterval(() => {
  cameras.forEach(pollCamera);
}, 1000 / 30);
