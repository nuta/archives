import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import Device from './device';

export default class MockDevice extends Device {
  updateOS(imagePath) {
    console.log('os image:', fs.readFileSync(imagePath, { encoding: 'utf-8' }))
    fs.renameSync(imagePath, path.join(os.homedir(), '.makestack/mock-os-update-test.img'))
  }
}
