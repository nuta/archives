use wasm_bindgen::prelude::*;
use wasm_bindgen::{Clamped, JsCast};
use web_sys::ImageData;

#[macro_use]
extern crate log;

#[cfg(feature = "wee_alloc")]
#[global_allocator]
static ALLOC: wee_alloc::WeeAlloc = wee_alloc::WeeAlloc::INIT;

use kazari::canvas::Canvas;
use kazari::client::Window;
use kazari::server::{Input, KeyCode, KeyState, PointerButton, PointerButtonState, PointerMove};
use kazari::standalone::Standalone;
use kazari::wl::protocols::common::wl_keyboard;
use kazari::wl::protocols::common::EventSet;
use kazari::wl::{Interface, ObjectId};
use kazari::{Color, FillStyle, FontFamily, FontSize, FontStyle, Position, RectArea, RectSize};
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

fn convert_keycode(code: &str) -> KeyCode {
    match code {
        "KeyA" => KeyCode::KeyA,
        "KeyB" => KeyCode::KeyB,
        "KeyC" => KeyCode::KeyC,
        "KeyD" => KeyCode::KeyD,
        _ => KeyCode::Enter,
    }
}

fn canvas2image(canvas: &Canvas) -> ImageData {
    ImageData::new_with_u8_clamped_array(Clamped(canvas.buffer()), canvas.size().width as u32)
        .unwrap()
}

/// Handles (wayland) events from the server like keyboard input.
fn handle_window_event(
    active_surface: &Rc<RefCell<Option<ObjectId>>>,
    window: Option<&mut Window>,
    ev: EventSet,
) {
    match ev {
        EventSet::WlKeyboard(_, ev) => match (window, ev) {
            (_, wl_keyboard::Event::Enter { surface, .. }) => {
                info!("enter keyboard: {:?}", surface.id());
                *active_surface.borrow_mut() = Some(surface.id());
            }
            (Some(window), wl_keyboard::Event::Key { key, .. }) => {
                window.canvas_mut().draw_rect(
                    Position::new(32, 32),
                    RectSize::new(32, 32),
                    Some(FillStyle {
                        color: Color::Rgba8888 {
                            r: 255,
                            g: 255,
                            b: 255,
                            a: 255,
                        },
                    }),
                    None,
                );
                window.canvas_mut().draw_text(
                    (key as u8 as char).to_string().as_str(),
                    FontSize::Normal,
                    FontFamily::Default,
                    FontStyle::Regular,
                    Position::new(32, 32),
                    Color::Rgba8888 {
                        r: 0,
                        g: 0,
                        b: 0,
                        a: 255,
                    },
                );
                window
                    .damage(RectArea::new(Position::new(0, 0), RectSize::new(100, 100)))
                    .unwrap();
                window.commit().unwrap();
            }
            _ => {}
        },
        _ => {}
    }
}

macro_rules! add_event_listener {
    ($gui:expr, $windows:expr, $active_surface:expr, $canvas:expr, $canvas_context:expr, $attach_event_to:expr, $event_name:expr, $input:expr) => {{
        let gui = $gui.clone();
        let windows = $windows.clone();
        let active_surface = $active_surface.clone();
        let context = $canvas_context.clone();
        let closure = Closure::wrap(Box::new(move |event| {
            let mut gui = gui.borrow_mut();
            let mut windows = windows.borrow_mut();
            // Send the user input to the Kazari server. It will move the mouse cursor,
            // notify the keyboard input to the client, etc...
            gui.process_input($input(event), |ev| {
                let window = active_surface
                    .borrow()
                    .map(|id| windows.get_mut(&id).unwrap());
                // The client has received a new event from the server. Let's handle it.
                handle_window_event(&active_surface, window, ev)
            });
            // The screen may be updated. Update the contents in `<canvas>`.
            gui.render(|canvas| {
                context
                    .put_image_data(&canvas2image(canvas), 0., 0.)
                    .unwrap();
            });
        }) as Box<dyn FnMut(_)>);
        $attach_event_to
            .add_event_listener_with_callback($event_name, closure.as_ref().unchecked_ref())?;
        closure.forget();
    }};
}

#[wasm_bindgen(start)]
pub fn main_js() -> Result<(), JsValue> {
    #[cfg(debug_assertions)]
    console_error_panic_hook::set_once();
    wasm_logger::init(wasm_logger::Config::new(log::Level::Trace));
    info!("starting Kazari on Web...");

    // Look for <canvas id="canvas">
    let document = web_sys::window().unwrap().document().unwrap();
    let canvas = document.get_element_by_id("canvas").unwrap();
    let canvas: web_sys::HtmlCanvasElement = canvas
        .dyn_into::<web_sys::HtmlCanvasElement>()
        .map_err(|_| ())
        .unwrap();
    let context = Rc::new(
        canvas
            .get_context("2d")
            .unwrap()
            .unwrap()
            .dyn_into::<web_sys::CanvasRenderingContext2d>()
            .unwrap(),
    );

    // Initialize Kazari.
    let mut gui = Standalone::new(640, 480);

    // Create a window on Kazari.
    let mut windows: HashMap<ObjectId, Window> = HashMap::new();
    let active_surface = Rc::new(RefCell::new(None));
    for i in 0..1 {
        let mut window = gui.create_window(150, 150).unwrap();
        window.enable_keyboard().unwrap();
        window.set_title(&format!("New Window {}", i)).unwrap();
        windows.insert(window.surface().id(), window);
    }

    // Draw the first screen.
    gui.render(|canvas| {
        context
            .put_image_data(&canvas2image(canvas), 0., 0.)
            .unwrap();
    });

    // Register event listeners to receive user inputs.
    let gui = Rc::new(RefCell::new(gui));
    let windows = Rc::new(RefCell::new(windows));
    add_event_listener!(
        gui,
        windows,
        active_surface,
        canvas,
        context,
        canvas,
        "mousemove",
        |event: web_sys::MouseEvent| {
            Input::PointerMove(PointerMove::Absolute {
                new_pos: Position::new(event.offset_x() as usize, event.offset_y() as usize),
            })
        }
    );
    add_event_listener!(
        gui,
        windows,
        active_surface,
        canvas,
        context,
        canvas,
        "mousedown",
        |event: web_sys::MouseEvent| {
            Input::PointerButton {
                state: PointerButtonState::Pressed,
                button: match event.button() {
                    0 => PointerButton::Left,
                    2 => PointerButton::Right,
                    _ => unimplemented!(),
                },
            }
        }
    );
    add_event_listener!(
        gui,
        windows,
        active_surface,
        canvas,
        context,
        canvas,
        "mouseup",
        |event: web_sys::MouseEvent| {
            Input::PointerButton {
                state: PointerButtonState::Released,
                button: match event.button() {
                    0 => PointerButton::Left,
                    2 => PointerButton::Right,
                    _ => unimplemented!(),
                },
            }
        }
    );
    add_event_listener!(
        gui,
        windows,
        active_surface,
        canvas,
        context,
        document,
        "keydown",
        |event: web_sys::KeyboardEvent| {
            Input::Keyboard {
                state: KeyState::Pressed,
                code: convert_keycode(&event.code()),
            }
        }
    );
    add_event_listener!(
        gui,
        windows,
        active_surface,
        canvas,
        context,
        document,
        "keyup",
        |event: web_sys::KeyboardEvent| {
            Input::Keyboard {
                state: KeyState::Released,
                code: convert_keycode(&event.code()),
            }
        }
    );

    Ok(())
}
