#!/usr/bin/env node

function grid(x, y, nu, nv) {
  const layout = [];
  for (let v = 0; v < nv; v++) {
    for (let u = 0; u < nu; u++) {
      layout.push({
        point: [x + (v % 2 == 0 ? nu - u - 1 : u), 0, y + v]
      });
    }
  }
  return layout;
}

const layout = grid(0, 0, 12, 10);

console.log(JSON.stringify(layout));
