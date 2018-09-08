import * as yaml from "js-yaml";
import * as highlightjs from "highlight.js";
import * as Markdown from "markdown-it";
const MarkdownKaTeX = require("@iktakahiro/markdown-it-katex");
const MarkdownEmoji = require("markdown-it-emoji");
const MarkdownFootnote = require("markdown-it-footnote");
const MarkdownCjkBreaks = require("markdown-it-cjk-breaks");
const twemoji = require("twemoji");

export type FrontMatter = { [name: string]: any };
export interface Slide {
    index: number, // 1-origin
    html: string,
};

export class SlideRanges {
    private ranges: number[];

    constructor(ranges: number[]) {
        this.ranges = ranges;
    }

    public getIndexByLine(line: number): number {
        let current = 0;
        for (const range of this.ranges) {
            if (line < range) {
                return current;
            }

            current++;
        }

        return 0;
    }
}

export class RenderedMarkdown {
    public front: FrontMatter;
    public slides: Slide[];
    private ranges: SlideRanges;

    constructor(front: FrontMatter, slides: Slide[], ranges: SlideRanges) {
        this.front = front;
        this.slides = slides;
        this.ranges = ranges;
    }

    public html(index: number): string {
        return this.slides[index].html;
    }

    public htmlByLine(line: number): string {
        return this.html(this.ranges.getIndexByLine(line));
    }
}

const SLIDE_HEADER = `
<ul class="error"></ul>
<p class="page-number"></p>
`;

export function render(text: string): RenderedMarkdown {
    let lines = text.split("\n");
   const endOfFront = 1 + lines.slice(1).indexOf("---");
    if (lines[0] !== "---" || endOfFront == -1) {
        throw new Error("Failed to locate the front matter");
    }

    // Parse front matter.
    const front = yaml.safeLoad(lines.slice(1, endOfFront).join("\n"));

    // Split into slides.
    const slideTexts = lines
        .slice(1 + endOfFront)
        .join("\n")
        .trim()
        .split(/\n----*[^\n]*\n/)
        .filter(page => page.length > 0);

    const md = new Markdown({
        highlight(str: string, lang?: string) {
            if (lang) {
                return highlightjs.highlight(lang, str).value;
            }

            return str;
        }
    });

    // Enable KaTeX.
    md.use(MarkdownKaTeX, { throwOnError: true });

    // Remove unwanted spaces in CJK sentences.
    md.use(MarkdownCjkBreaks);

    // Remove unwanted spaces in CJK sentences.
    md.use(MarkdownFootnote);

    // Enable Emoji.
    md.use(MarkdownEmoji);
    md.renderer.rules.emoji = (token: Markdown.Token[], index: number) => {
      return twemoji.parse(token[index].content, { folder: "svg", ext: ".svg" });
    };

    // Convert markdown texts into html.
    const ranges: number[] = [];
    const slides = slideTexts.map((slideText, index) => {
        ranges.push((ranges[ranges.length - 1] || 0) + slideText.split("\n").length + endOfFront);

        return {
            index: index + 1,
            html: SLIDE_HEADER + md.render(slideText)
        };
    });

    const range = new SlideRanges(ranges);
    return new RenderedMarkdown(front, slides, range);
}
