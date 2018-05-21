export class FatalError extends Error {
}

export class APIError extends Error {
    status: number;
    response?: any;

    constructor(message: string, status: number, response: any) {
        super(message);
        this.status = status;
        this.response = response;
    }
}
