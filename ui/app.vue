<template>
    <div id="app" class="supershow" :data-theme="theme">
        <div class="wallpaper">
            <div class="presenter" v-if="presenterView">
                <button @click="exitPresenterView">exit</button>
                <button @click="moveFromPresenterView(-1)">prev</button>
                <button @click="moveFromPresenterView(1)">next</button>
            </div>
            <div class="container" v-else>
                <button @click="enterPresenterView">PRESENT</button>
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
            presentation: null,
            presentationCon: null,
            presenterView: false,
            isPresentationAvailable: false,
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
        move(offset) {
            this.moveTo(this.index + offset);
        },
        loadTheme(theme) {
            this.theme = theme;
        },
        setupShortcuts() {
            window.onkeyup = (ev) => {
                switch (ev.code) {
                    case "Enter":
                        this.move(1);
                        break;
                    case "Space":
                        this.move(1);
                        break;
                    case "ArrowUp":
                        this.move(-1);
                        break;
                    case "ArrowDown":
                        this.move(1);
                        break;
                    case "ArrowLeft":
                        this.move(-1);
                        break;
                    case "ArrowRight":
                        this.move(1);
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
        },
        enterPresenterView() {
            if (this.isPresentationAvailable) {
                this.presentation.start().then((con) => {
                    // We're in the presenter view.

                    con.onclose = () => {
                        // The presentation view is being closed. Go
                        // back from the presenter view.
                        this.presenterView = false;
                        this.presentationCon = null;
                    };

                    this.presenterView = true;
                    this.presentationCon = con;
                });
            }
        },
        exitPresenterView() {
            // Close the presentation view.
            this.presentationCon.terminate();
            this.presentationCon.close();
        },
        moveFromPresenterView(offset) {
            this.presentationCon.send(JSON.stringify({
                action: "move",
                offset
            }));
        },
        async setupPresenterView() {
            if (navigator.presentation.receiver) {
                navigator.presentation.receiver.connectionList.then((list) => {
                    list.connections.map((con) => {
                        con.onmessage = ({ data }) => {
                            // We're in the presentation view and have received
                            // a message from the presenter view.
                            const { action, offset } = JSON.parse(data);
                            switch (action) {
                                case "move":
                                    this.move(offset);
                                    break;
                            }
                        }
                    })
                });
            }
        },
        async setupPresentationRequest() {
            const onavailchange = (state) => {
                console.log("Presentation Availability:", state);
                this.isPresentationAvailable = state;
            }

            const request = new PresentationRequest("/");
            const avail = await request.getAvailability();
            avail.onchange = function() { onavailchange(this.value) };
            onavailchange(avail.value);
            this.presentation = request;
        }
    },
    async mounted() {
        if (WEBPACK_MODE === "development") {
            this.loadMarkdown(require("raw-loader!../examples/test.md"));
        } else {
            // TODO: production
        }

        this.moveTo(0);
        this.setupShortcuts();
        await this.setupPresentationRequest();
        this.setupPresenterView();

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
