import * as path from "path";
import * as fs from "fs";
import * as mustache from "mustache";
import * as puppeteer from "puppeteer";
import { render } from "./render";

export async function generatePdf(file: string, output: string) {
    const templateFile = path.resolve(__dirname, "../dist/ui/index.html");
    const template = fs.readFileSync(templateFile, { encoding: "utf-8" });
    const htmlFile = path.resolve(file + ".html");
    const md = fs.readFileSync(file, { encoding: "utf-8" });

    let slidesHtml = "";
    const rendered = render(md);
    const size = (rendered.front.size || "4:3").replace(":", "x");
    for (const slide of rendered.slides) {
        slidesHtml += `<div class="slide size-${size}">${slide.html}</div>`;
    }

    const html = mustache.render(template, {
        title: rendered.front.title || "No title",
        theme: rendered.front.theme || "simple",
        body: slidesHtml,
    });
    fs.writeFileSync(htmlFile, html);
    const uri = "file://" + htmlFile;

    const browser = await puppeteer.launch();
    const page = await browser.newPage();
    page.on('console', msg => console.log('chrome:', msg.text()));
    await page.goto(uri, { waitUntil: "networkidle0" });
    const width  = size == "16x9" ? "1210px" : "908px";
    const height = size == "16x9" ? "681px" : "681px";
    await page.pdf({ path: output, width, height });
    await browser.close();
}
