import * as caporal from "caporal";

export interface Args { [name: string]: any; }
export interface Opts { [name: string]: any; }

export function constructEnvOption(defaultValue: "development" | "production") {
    return {
        name: "--env",
        desc: "The environment.",
        default: defaultValue,
        validator: ["development", "production"],
    }
}

export type Validator
    =  RegExp
    | number // Caporal flags
    | ((arg: string) => boolean)
    ;

export interface ArgDefinition {
    name: string;
    desc: string;
    validator?: Validator;
    default?: any;
}

export interface OptDefinition {
    name: string;
    desc: string;
    validator?: Validator;
    default?: any;
    required?: boolean;
}

export abstract class CommandBase {
    public static command: string;
    public static desc: string;
    public static args: ArgDefinition[];
    public static opt: OptDefinition[];
    public abstract async run(args: Args, opts: Opts): Promise<void>;
}
