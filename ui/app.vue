<template>
    <div id="app" class="supershow" :data-theme="theme" :class="{ ['disable-cursor']: laser }">
        <div class="wallpaper">
            <div class="presenter" v-if="presenterView">
                <button @click="exitPresenterView">exit</button>
                <button @click="moveFromPresenterView(-1)">prev</button>
                <button @click="moveFromPresenterView(1)">next</button>
            </div>
            <div class="container" v-else ref="container">
                <div class="slides">
                    <div v-for="slide in slides" :key="slide.index"
                     :class="[ (index === slide.index) ? 'current' : '', 'size-' + size]"
                     class="slide" v-html="slide.html"></div>
                </div>
                <div class="bottom-bar">
                    <div class="left-buttons">
                        <i class="mdi mdi-crosshairs-gps button" @click="toggleLaser"
                         :class="{ enabled: laser }"></i>
                    </div>
                    <div></div>
                    <div class="right-buttons">
                        <i class="mdi mdi-fullscreen button" @click="enterFullscreen"></i>
                        <i class="mdi mdi-cast button" @click="enterPresenterView"></i>
                    </div>
                </div>
                <div class="page-number">
                    {{ index + 1 }} / {{ slides.length }}
                </div>
                <div class="laser-light" ref="laserLight" v-show="laser"></div>
            </div>
        </div>
    </div>
</template>

<script>
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
            laser: false,
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
        toggleLaser() {
            this.laser = !this.laser;
            document.onmousemove = (ev) => {
                const x = ev.pageX;
                const y = ev.pageY;
                this.$refs.laserLight.style.top = y + "px";
                this.$refs.laserLight.style.left = x + "px";
            }
        },
        enterFullscreen() {
            const container = this.$refs.container;
            if (container.requestFullscreen) {
                container.requestFullscreen();
            } else if (container.webkitRequestFullscreen) {
                container.webkitRequestFullscreen();
            } else if (container.mozRequestFullscreen) {
                container.mozRequestFullscreen();
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
@import "~highlight.js/styles/solarized-light.css";
@import "~katex/dist/katex.min.css";
$mdi-font-path: "~@mdi/font/fonts";
@import "~@mdi/font/scss/materialdesignicons.scss";
@import "./themes/simple.scss";

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
        width: 100vw;
        height: 100vh;

        .slides {
            height: 100%;
        }

        .slide {
            display: none;
            box-sizing: border-box;
            margin: 0 auto;
            height: 100%;

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
        }

        .page-number {
            font-size: 20px;
            position: absolute;
            bottom: 0;
            text-align: center;
            width: 100%;
        }

        .bottom-bar {
            width: 100%;
            display: flex;
            position: absolute;
            justify-content: space-between;
            bottom: 0;
            color: #ffffff;
            background: rgba(0, 0, 0, 0.30);
            padding: 0px 20px;
            box-sizing: border-box;

            .button {
                font-size: 28px;
                padding: 3px 5px;

                &.enabled {
                    color: #ff3344;
                }

                &:hover {
                    cursor: pointer;
                    background: rgba(0, 0, 0, 0.32);
                    border-radius: 3px;
                    transition: 0.1s ease-in;
                }
            }
        }
    }

    &.disable-cursor {
        cursor: none;
    }

    .laser-light {
        $light-size: 14px;
        position: absolute;
        background: rgba(255, 0, 0, 0.75);
        box-shadow: 0px 0px 15px 4px rgba(255, 0, 0, 0.7);
        height: $light-size;
        width: $light-size;
        border-radius: $light-size / 2;
        pointer-events: none;
    }
}
</style>
