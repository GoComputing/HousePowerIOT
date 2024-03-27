import { HttpClient, HttpHeaders } from "@angular/common/http";
import { Injectable } from "@angular/core";
import { timer } from 'rxjs';
import { mergeMap, map } from 'rxjs/operators'
import { secrets } from '../../environments/secrets'
import { environment } from '../../environments/environment'

// Useful resources
//   Observable/observer pattern: https://www.linkedin.com/pulse/observables-observer-rxjs-mithun-pal
//   Observable implementation: https://angular.io/guide/observables-in-angular
//   https://stackoverflow.com/questions/47236963/no-provider-for-httpclient
//   On device debug: https://stackoverflow.com/questions/38744809/ionic-2-how-can-i-get-console-messages-from-android-device
//   Allow http requests: https://stackoverflow.com/questions/75171259/android-virtual-device-error-this-request-has-been-blocked-the-content-must-be


@Injectable({
    providedIn: 'root'
})
export class EnergyObservable {
    constructor(private _http: HttpClient) {
        
    }

    getData(period: number) {
        const authToken = secrets.influxdb_api_key;

        // Set up the headers
        const headers = new HttpHeaders({
            'Authorization': `Token ${authToken}`,
            'Content-Type': 'application/json',
        });

        // Define the Flux query
        const fluxQuery = JSON.stringify({
            query: "from(bucket:\"Sensors\") |> range(start: -1) |> filter(fn: (r) => r._field == \"power_W\") |> last()"
        });

        const api_request = this._http.post(environment.influxdb_url, fluxQuery, { headers, responseType: 'text' })
            .pipe(map(response => this.csvJSON(response)));

        const source = timer(0, period);
        return source.pipe(
            mergeMap(value => {
                return api_request;
            })
        );
    }

    public csvJSON(csv: string) {
        var lines = csv.split("\n");
        var result = [];
        var headers = lines[0].split(",");

        for (var i = 1; i < lines.length; i++) {
            const obj: any = {};
            var currentline = lines[i].split(",");

            for (var j = 0; j < headers.length; j++) {
                obj[headers[j]] = currentline[j];
            }

            result.push(obj);
        }

        //return result; //JavaScript object
        return result; //JSON
    }
}
