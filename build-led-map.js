#!/usr/bin/nodejs

// [ OPC Channel, First OPC Pixel, First output pixel, Pixel count ]
//     Map a contiguous range of pixels from the specified OPC channel to the current device
//     For Fadecandy devices, output pixels are numbered from 0 through 511. Strand 1 begins at index 0, strand 2 begins at index 64, etc.

const GRID_SIZE_X = 2 * 12;
const GRID_SIZE_Y = 2 * 10;

const makePanel = (sx, sy, si) => {
  const map = [];
  for (let y = sy + 4, i = si; y >= sy; --y) {
    map.push([0, y * GRID_SIZE_X + sx, i, y % 2 == 0 ? 12 : -12]);
    i += 12;
  }
  for (let y = sy + 5, i = si + 64; y < sy + 12; ++y) {
    map.push([0, y * GRID_SIZE_X + sx, i, y % 2 == 1 ? 12 : -12]);
    i += 12;
  }
  return map;
};

console.log(JSON.stringify(makePanel(0, 0, 0)));
