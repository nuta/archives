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
                <div v-for="slide in slides" :key="slide.index"
                 :class="[ (index === slide.index) ? 'current' : '', 'size-' + size]"
                 class="slide" v-html="slide.html"></div>
                <div class="page-number">{{ index + 1 }} / {{ slides.length }}</div>
            </div>
        </div>
    </div>
</template>

<script>
import "highlight.js/styles/solarized-light.css";
import "katex/dist/katex.min.css";
import "./themes/simple.scss";
import { render } from "render";

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
            const { front, slides } = render(text);
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
        page-break-after: always;

        .slide {
            display: none;

            @media print {
                display: block;
            }

            &.current {
                display: block;
            }

            .footnote-ref a {
                color: #333;
                font-size: 80%;
                text-decoration: none;

                &:hover {
                    cursor: initial;
                }
            }

            .footnotes-sep, .footnote-backref {
                display: none;
            }

            .footnotes {
                font-size: 70%;
            }

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
