import { Board } from "../types";
import { Esp32Board } from "./esp32";

export const board = new Esp32Board() as Board;
