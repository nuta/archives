import * as path from "path";
import * as fs from "fs";
import * as puppeteer from "puppeteer";
import { renderHtml } from "./render";

export async function generatePdf(file: string, output: string) {
    const htmlFile = path.resolve(file + ".html");
    const md = fs.readFileSync(file, { encoding: "utf-8" });
    const { html, front } = renderHtml(md);
    const uri = "file://" + htmlFile;
    fs.writeFileSync(htmlFile, html);

    const browser = await puppeteer.launch();
    const page = await browser.newPage();
    page.on('console', msg => console.log('chrome:', msg.text()));
    await page.goto(uri, { waitUntil: "networkidle0" });
    const width  = front.size == "16:9" ? "1210px" : "908px";
    const height = front.size == "16:9" ? "681px" : "681px";
    await page.pdf({ path: output, width, height });
    await browser.close();
}
