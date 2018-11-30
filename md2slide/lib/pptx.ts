const PptxGenJS = require("pptxgenjs");
import * as fs from "fs";
import { generatePdf } from "./pdf";
import { spawnSync } from "child_process";

export async function generatePptx(file: string, output: string) {
    const tmpPdf = file + ".pdf";
    const tmpPng = "supershow-tmp.png";

    // Generate a .png file.
    await generatePdf(file, tmpPdf);

    // Convert to .png files.
    const args =  ["-density", "300", "-quality", "100", "-resize", "x2048"];
    spawnSync("convert",[...args, tmpPdf, tmpPng]);

    // Create a new .pptx file.
    const pptx = new PptxGenJS();
    pptx.setLayout("LAYOUT_4x3");
    for (const slidePng of fs.readdirSync(".")) {
        if (slidePng.match(/^supershow-tmp-.*\.png$/)) {
            const slide = pptx.addNewSlide();
            slide.addImage({
                path: slidePng,
                x: 0,
                y: 0,
                w: 10,
                h: 7.5,
            });
        }
    }

    pptx.save(output);

    for (const slidePng of fs.readdirSync(".")) {
        if (slidePng.match(/^supershow-tmp-.*\.png$/)) {
            fs.unlinkSync(slidePng);
        }
    }
    fs.unlinkSync(tmpPdf);
}
