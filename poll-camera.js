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

let waiting = false;

const onPoll = () => {
  let readings = "";
  for (let i = 0; i < 8; ++i) {
    readings += (cameras[0].reading[i] && cameras[1].reading[i]) ? "1" : "0";
  }
  console.log(readings);
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

          camera.reading = Array(8);
          for (let i = 0; i < 8; ++i) {
            camera.reading[i] = !!json[i];
          }

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
