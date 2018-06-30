import * as path from "path";
import * as fs from "fs";
import { logger } from "./logger";
import { spawnSync } from "child_process";

export function exportTo(file: string, output: string) {
    logger.progress(`Exporting to ${file}`);
    const templateFile = path.resolve(__dirname, "../dist/ui/index.html");
    const template = fs.readFileSync(templateFile, { encoding: "utf-8" });
    const md = JSON.stringify(fs.readFileSync(file, { encoding: "utf-8" }));
    const inject = "<script> window.markdownText = " + md + ";</script>";
    const htmlFile = file + ".html";
    const html = template.replace("</body>", inject + "</body>");
    fs.writeFileSync(htmlFile, html);
    const uri = "file://" + htmlFile;

    const chrome = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome";
    logger.command("chrome --headless --print-to-pdf");
    const { error } = spawnSync(
        chrome,
        ["--headless", "--disable-gpu", "--print-to-pdf", uri],
        {
            stdio: "inherit"
        }
    );

    if (error) {
        throw new Error(`failed to run chrome`);
    }

    fs.unlinkSync(htmlFile);
    fs.copyFileSync("output.pdf", output);
    fs.unlinkSync("output.pdf");
}
