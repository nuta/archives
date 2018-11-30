import * as fs from "fs";
import * as path from "path";
import * as zlib from "zlib";
import { makedirs } from "./fsutils";

const SIGNATURE_FILE = Buffer.from([0x50, 0x4b, 0x03, 0x04]);
const SIGNATURE_CENTRAL_DIR = Buffer.from([0x50, 0x4b, 0x01, 0x02]);
const FILEHEADER_FLAGS = 0x06;
const FILEHEADER_COMPRESSION = 0x08;
const FILEHEADER_COMPRESSED_SIZE = 0x12;
const FILEHEADER_FILENAME_LENGTH = 0x1a;
const FILEHEADER_FIELD_LENGTH = 0x1c;
const FILEHEADER_FILENAME = 0x1e;
const DATA_DESCRIPTOR_LENGTH = 12;

type CompressionType = "plain" | "deflate" | "lzma" | "unknown";

interface Flags {
    encrypted: boolean;
    dataDescriptor: boolean;
}

interface LocalFileHeader {
    flags: Flags;
    compression: CompressionType;
    filename: string;
    dataOffset: number;
    dataLength: number;
}

function parseFlags(flags: number): Flags {
    return {
        encrypted:      (flags & (1 << 0)) === 1,
        dataDescriptor: (flags & (1 << 3)) === 1,
    };
}

function parseCompression(compression: number): CompressionType {
    switch (compression) {
        case 0: return "plain";
        case 8: return "deflate";
        case 14: return "lzma";
        default: return "unknown";
    }
}

function isCentralDirHeader(data: Buffer): boolean {
    return data.slice(0, 4).compare(SIGNATURE_CENTRAL_DIR) === 0;
}

function parseLocalFileHeader(data: Buffer): LocalFileHeader {
    // Verify the signature.
    if (data.slice(0, 4).compare(SIGNATURE_FILE) !== 0) {
        throw new Error("invalid zip file (bad signature)");
    }

    const flags = parseFlags(data.readUInt16LE(FILEHEADER_FLAGS));
    const compression = parseCompression(data.readUInt16LE(FILEHEADER_COMPRESSION));
    const filenameLength = data.readUInt16LE(FILEHEADER_FILENAME_LENGTH);
    const extraFieldLength = data.readUInt16LE(FILEHEADER_FIELD_LENGTH);
    const dataLength = data.readUInt32LE(FILEHEADER_COMPRESSED_SIZE);
    const filenameEndOffset = FILEHEADER_FILENAME + filenameLength;
    const filename = data.slice(FILEHEADER_FILENAME, filenameEndOffset).toString();
    const dataOffset = filenameEndOffset + extraFieldLength;

    return { flags, compression, filename, dataOffset, dataLength };
}

function extractFile(filename: string, compression: CompressionType, compressedData: Buffer, destDir: string) {
    let data;
    switch (compression) {
        case "deflate":
        data = zlib.inflateRawSync(compressedData);
        break;
        case "plain":
        data = compressedData;
        break;
        default:
        throw new Error(`unsupported file compression (filename=${filename})`);
    }

    makedirs(path.join(destDir, path.dirname(filename)));
    fs.writeFileSync(path.join(destDir, filename), data);
}

export function extract(zip: Buffer, destDir: string): string[] {
    const end = zip.length;
    const files = [];
    let offset = 0;
    while (offset < end && !isCentralDirHeader(zip.slice(offset))) {
        const { flags, compression, filename, dataOffset, dataLength } = parseLocalFileHeader(zip.slice(offset));
        const compressedData = zip.slice(offset + dataOffset, offset + dataOffset + dataLength);

        if (!filename.endsWith("/")) {
            extractFile(filename, compression, compressedData, destDir);
            files.push(filename);
        }

        const diff = dataOffset + dataLength + ((flags.dataDescriptor) ? DATA_DESCRIPTOR_LENGTH : 0);

        // Avoid infinite loop.
        if (diff === 0) {
            throw new Error(`invalid zip file (offset=${offset}, filename=${filename})`);
        }

        offset += diff;
    }

    return files;
}
