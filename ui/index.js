require("./main.scss");
require("./themes/simple.scss");

function getSlideTitle(slide) {
    return slide.querySelector("h1").textContent;
}

function addError(slide, message) {
    const container = slide.querySelector(".error");
    const li = document.createElement("li");
    li.innerText = message;
    container.appendChild(li);
}

function scaleFontSizeToFit(container, elem) {
    if (!elem) {
        return;
    }

    const maxSize = 40;
    const minSize = 25;

    // Temporarily disable wrapping to compute `elem.clientHeight` correctly.
    elem.style.whiteSpace = "nowrap";

    for (let i = maxSize; i >= minSize; i--) {
        elem.style.fontSize = `${i}px`;
        if (elem.clientWidth < container.clientWidth) {
            // The font size is small enough to fit.
            break;
        }
    }

    elem.style.whiteSpace = "normal";
}

window.onload = () => {
    const slides = document.querySelectorAll(".slide");

    for (const slide of slides) {
        const h1 = slide.querySelector("h1");
        scaleFontSizeToFit(slide, h1);
    }

    const threshold = 10;
    for (const slide of slides) {
        const maxHeight = slide.clientHeight;
        const maxWidth = slide.clientWIdth;
        console.log(slide.clientHeight, slide.scrollHeight);
        if (maxHeight < slide.scrollHeight - threshold
            || maxWidth < slide.scrollWidth - threshold) {
            addError(slide, "overflowed content");
            console.error(`overflowed content (in "${getSlideTitle(slide)}")`);
        }
    }

    document.querySelector("#slides").style.visibility = "visible";
}
