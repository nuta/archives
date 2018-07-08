import * as yaml from "js-yaml";
import * as highlightjs from "highlight.js";
import * as Markdown from "markdown-it";
const MarkdownKaTeX = require("markdown-it-katex");
const MarkdownEmoji = require("markdown-it-emoji");
const MarkdownFootnote = require("markdown-it-footnote");
const MarkdownCjkBreaks = require("markdown-it-cjk-breaks");
const twemoji = require("twemoji");

export function render(text: string) {
    let lines = text.split("\n");
   const endOfFront = 1 + lines.slice(1).indexOf("---");
    if (lines[0] !== "---" || endOfFront == -1) {
        throw new Error("Failed to locate the front matter");
    }

    // Parse front matter.
    const front = yaml.safeLoad(lines.slice(1, endOfFront).join("\n"));

    // Split into slides.
    // FIXME: Support blank slideTexts (i.e. "---\n---").
    const slideTexts = lines
        .slice(1 + endOfFront)
        .join("\n")
        .split(/\n---(-*\s*)\n/)
        .filter(page => page.length > 0);

    const md = new Markdown({
        highlight(str: string, lang?: string) {
            if (lang) {
                try {
                    return highlightjs.highlight(lang, str).value;
                } catch (e) {
                    console.error(e);
                }
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
    const slides = slideTexts.map((slideText, index) => {
        return {
            index,
            html: md.render(slideText)
        };
    });

    return { front, slides };
}
