<template>
    <div id="app" class="supershow" :data-theme="theme">
        <div class="wallpaper">
            <div class="container">
                <div v-for="slide in slides" v-show="index === slide.index" :key="slide.index"
                 class="slide" :class="'size-' + size" v-html="slide.html"></div>
                <div class="page-number">{{ index + 1 }} / {{ slides.length }}</div>
            </div>
        </div>
    </div>
</template>

<script>
import yaml from "js-yaml";
import highlightjs from "highlight.js";
import "highlight.js/styles/solarized-light.css";
import "katex/dist/katex.min.css";
import "./themes/simple.scss";
import Markdown from "markdown-it";
import MarkdownKaTeX from "markdown-it-katex";
import MarkdownEmoji from "markdown-it-emoji";
import twemoji from "twemoji";

function parseMarkdown(text) {
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
        highlight(str, lang) {
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
    md.use(MarkdownKaTeX);

    // Enable Emoji.
    md.use(MarkdownEmoji);
    md.renderer.rules.emoji = (token, index) => {
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

export default {
    data() {
        return {
            theme: null,
            index: 0,
            slides: [],
            html: "",
            size: null,
        }
    },
    methods: {
        moveTo(index) {
            const slide = this.slides[index];
            if (!slide) {
                return;
            }

            this.index = index;
            this.html = slide.html;
        },
        loadTheme(theme) {
            this.theme = theme;
        },
        setupShortcuts() {
            window.onkeyup = (ev) => {
                switch (ev.code) {
                    case "Enter":
                        this.moveTo(this.index + 1);
                        break;
                    case "Space":
                        this.moveTo(this.index + 1);
                        break;
                    case "ArrowUp":
                        this.moveTo(this.index - 1);
                        break;
                    case "ArrowDown":
                        this.moveTo(this.index + 1);
                        break;
                    case "ArrowLeft":
                        this.moveTo(this.index - 1);
                        break;
                    case "ArrowRight":
                        this.moveTo(this.index + 1);
                        break;
                }
            }
        },
        loadMarkdown(text) {
            const { front, slides } = parseMarkdown(text);
            document.title = front.title || "slide";
            this.slides = slides;
            this.size = (front.size || "4:3").replace(":", "x");
            this.loadTheme(front.theme || "simple");
        },
        watchChanges() {
            console.log("Connecting to the local server...");
            const ws = new WebSocket("ws://" + location.host);
            ws.onmessage = (ev) => {
                console.log("Applying changes.");
                const { text } = JSON.parse(ev.data);
                this.loadMarkdown(text);
            }
        }
    },
    mounted() {
        if (WEBPACK_MODE === "development") {
            this.loadMarkdown(require("raw-loader!../examples/test.md"));
        } else {
            // TODO: production
        }

        this.moveTo(0);
        this.setupShortcuts();

        if (["localhost", "127.0.0.1"].includes(location.hostname)) {
            this.watchChanges();
        }
    }
}
</script>

<style lang="scss">
html, body {
    margin: 0;
    padding: 0;
    height: 100%;
    width: 100%;
}

.supershow {
    height: 100%;
    width: 100%;
    margin: 0;
    padding: 0;

    .wallpaper {
        height: 100%;
        width: 100%;
        margin: 0;
        padding: 0;
    }

    .container {
        .slide {
            box-sizing: border-box;
            margin: 0 auto;
            width: 100vw;
            height: 100vh;
        }

        $page-number-height: 20px;
        .page-number {
            font-size: 18px;
            height: $page-number-height;
            bottom: $page-number-height + 10px;
            left: 10px;
            position: absolute;
            text-align: center;
        }
    }
}
</style>
