#!/usr/bin/nodejs

'use strict';

// [ OPC Channel, First OPC Pixel, First output pixel, Pixel count ]
//     Map a contiguous range of pixels from the specified OPC channel to the current device
//     For Fadecandy devices, output pixels are numbered from 0 through 511. Strand 1 begins at index 0, strand 2 begins at index 64, etc.

const PANELS_X = 2;
const PANELS_Y = 2;
const GRID_SIZE_X = PANELS_X * 12;
const GRID_SIZE_Y = PANELS_Y * 10;

const makePanel = (sx, sy, si, flip_x, flip_y) => {
  const map = [];
  for (let y = 4, i = si; y >= 0; --y) {
    const m = [0, ((flip_y ? (9 - y) : y) + sy) * GRID_SIZE_X + sx, i, 12];
    if (y % 2 == (flip_x ? 0 : 1)) {
      m[2] += 11;
      m[3] *= -1;
    }
    map.push(m);
    i += 12;
  }
  for (let y = 5, i = si + 64; y < 10; ++y) {
    const m = [0, ((flip_y ? (9 - y) : y) + sy) * GRID_SIZE_X + sx, i, 12];
    if (y % 2 == (flip_x ? 1 : 0)) {
      m[2] += 11;
      m[3] *= -1;
    }
    map.push(m);
    i += 12;
  }
  return map;
};

const makePanelGroup = (sx, sy) => {
  let map = [];
  let si = 0;
  for (let x = 0; x < PANELS_X; ++x) {
    for (let y = 0; y < PANELS_Y; ++y) {
      map = map.concat(makePanel(sx + 12 * (PANELS_X - x - 1), sy + 10 * y, si, x == 1, x == 0));
      si += 128;
    }
  }
  return map;
};

for (let y = 0; y < 2; ++y) {
  const map = makePanelGroup(0, GRID_SIZE_Y * y);
  console.log(`Group ${y}: `, JSON.stringify(map), "\n");
}
