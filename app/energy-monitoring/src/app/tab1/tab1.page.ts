import { AfterViewInit, Component, ElementRef, ViewChild } from '@angular/core';
import { Chart } from 'chart.js/auto';
import { EnergyObservable } from '../energy-observable/energy-observable.service';

@Component({
  selector: 'app-tab1',
  templateUrl: 'tab1.page.html',
  styleUrls: ['tab1.page.scss']
})
export class Tab1Page implements AfterViewInit {

    // Importing ViewChild. We need @ViewChild decorator to get a reference to the local variable 
    // that we have added to the canvas element in the HTML template.
    @ViewChild('gaugeCanvas') private gaugeCanvas: ElementRef;

    gaugeChart: any;

    constructor(private _energyobservable: EnergyObservable) { }

    // When we try to call our chart to initialize methods in ngOnInit() it shows an error nativeElement of undefined. 
    // So, we need to call all chart methods in ngAfterViewInit() where @ViewChild and @ViewChildren will be resolved.
    ngAfterViewInit() {
        this.gaugeChartMethod();

        this._energyobservable.getPeriodicLastData(5000).subscribe(
            response => { this.updateGaugeChart(response[0]._value); },
            error => { console.log(error) }
        );
    }

    gaugeChartMethod() {
        this.gaugeChart = new Chart(this.gaugeCanvas.nativeElement, {
            type: 'doughnut',
            data: {
                labels: ["Consumiendo", "Margen restante"],
                datasets: [{
                    label: 'Potencia (W)',
                    data: [],
                    backgroundColor: []
                }]
            },
            options: {
                rotation: 270, // start angle in degrees
                circumference: 180, // sweep angle in degrees
            }
        });
    }

    updateGaugeChart(value: number) {
        const max_power = 4000;

        if(value > max_power)
            value = max_power;

        this.gaugeChart.data.datasets = [{
            label: 'Potencia (W)',
            data: [value, max_power-value],
            backgroundColor: [
                "rgb(235, 212, 120)",
                "rgb(80, 80, 80)"
            ]
        }];
        this.gaugeChart.options.animation = false;
        this.gaugeChart.update();
    }
}
