import * as path from "path";
import * as fs from "fs";
import * as puppeteer from "puppeteer";
import { render } from "./render";

export async function generatePdf(file: string, output: string) {
    const templateFile = path.resolve(__dirname, "../dist/ui/index.html");
    const template = fs.readFileSync(templateFile, { encoding: "utf-8" });
    const htmlFile = path.resolve(file + ".html");
    const md = fs.readFileSync(file, { encoding: "utf-8" });

    let slidesHtml = "";
    const rendered = render(md);
    for (const slide of rendered.slides) {
        slidesHtml += "<div class=\"slide\">" + slide.html + "</div>";
    }

    let html = template.replace("__TITLE__", rendered.front.title);
    html = html.replace("__THEME__", rendered.front.theme || "simple");
    html = html.replace("__HTML__", slidesHtml);
    fs.writeFileSync(htmlFile, html);
    const uri = "file://" + htmlFile;

    const browser = await puppeteer.launch();
    const page = await browser.newPage();
    page.on('console', msg => console.log('chrome:', msg.text()));
    await page.goto(uri, { waitUntil: "networkidle0" });
    const width  = "908px";
    const height = "681px";
    await page.pdf({ path: output, width, height });
    await browser.close();
}
