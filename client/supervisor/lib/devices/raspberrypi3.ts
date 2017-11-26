import * as fs from 'fs';
import { DeviceBase } from './device_base';

export class Device extends DeviceBase {
  updateOS(imagePath) {
    fs.renameSync(imagePath, '/boot/kernel7.img')
  }
}
