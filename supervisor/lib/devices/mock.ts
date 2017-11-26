import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import { DeviceBase } from './device_base';

export class Device extends DeviceBase {
  updateOS(imagePath) {
    console.log('os image:', fs.readFileSync(imagePath, { encoding: 'utf-8' }))
    fs.renameSync(imagePath, path.join(os.homedir(), '.makestack/mock-os-update-test.img'))
  }
}
