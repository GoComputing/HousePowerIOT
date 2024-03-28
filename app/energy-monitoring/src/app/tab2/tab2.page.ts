import { AfterViewInit, Component, ElementRef, ViewChild } from '@angular/core';
import { Chart } from 'chart.js/auto';
import { EnergyObservable } from '../energy-observable/energy-observable.service';

@Component({
  selector: 'app-tab2',
  templateUrl: 'tab2.page.html',
  styleUrls: ['tab2.page.scss']
})
export class Tab2Page implements AfterViewInit {

    // Importing ViewChild. We need @ViewChild decorator to get a reference to the local variable 
    // that we have added to the canvas element in the HTML template.
    @ViewChild('lineCanvas') private lineCanvas: ElementRef;

    lineChart: any;
    lineChartData: number[];
    lineChartLabels: number[];

    constructor(private _energyobservable: EnergyObservable) { }

    // When we try to call our chart to initialize methods in ngOnInit() it shows an error nativeElement of undefined. 
    // So, we need to call all chart methods in ngAfterViewInit() where @ViewChild and @ViewChildren will be resolved.
    ngAfterViewInit() {
        this.lineChartMethod();
    }

    lineChartMethod() {
        this.lineChart = new Chart(this.lineCanvas.nativeElement, {
            type: 'line',
            data: {
                labels: this.lineChartLabels,
                datasets: [
                    {
                        label: 'Potencia usada (W)',
                        fill: false,
                        tension: 0,
                        backgroundColor: 'rgba(75,192,192,0.4)',
                        borderColor: 'rgba(75,192,192,1)',
                        borderCapStyle: 'butt',
                        borderDash: [],
                        borderDashOffset: 0.0,
                        borderJoinStyle: 'miter',
                        pointBorderColor: 'rgba(75,192,192,1)',
                        pointBackgroundColor: '#fff',
                        pointBorderWidth: 1,
                        pointHoverRadius: 5,
                        pointHoverBackgroundColor: 'rgba(75,192,192,1)',
                        pointHoverBorderColor: 'rgba(220,220,220,1)',
                        pointHoverBorderWidth: 2,
                        pointRadius: 1,
                        pointHitRadius: 10,
                        data: this.lineChartData,
                        spanGaps: false,
                    }
                ]
            }
        });

        this._energyobservable.getData('-1d', '30m').subscribe(
            response => { this.updateLineChart(response) },
            error => { console.log(error) }
        );
    }

    updateLineChart(response: any[]) {
        this.lineChartData = response.map(sample => sample._value);
        this.lineChartLabels = response.map(sample => new Date(sample._time).getUTCHours());

        this.lineChartData = this.lineChartData.filter(Boolean);
        this.lineChartLabels = this.lineChartLabels.filter(sample => !isNaN(sample));

        this.lineChart.data.labels = this.lineChartLabels;
        this.lineChart.data.datasets[0].data = this.lineChartData;

        this.lineChart.update();
    }
}
