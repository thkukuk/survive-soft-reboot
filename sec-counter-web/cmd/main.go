// Copyright 2024 Thorsten Kukuk
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package main

import (
	"bytes"
	//"fmt"
	"log"
	"net/http"
	"os"
	"os/exec"
	"os/signal"
	"strconv"
	"strings"
	"syscall"
	"text/template"
	"time"
	"math"

        "github.com/spf13/cobra"
)
var (
        Version="unreleased"
	indexTemplate *template.Template
	dir string
	Seconds=0
	StartTime time.Time
)


func main() {
// secCounterCmd represents the secCounter command
	secCounterCmd := &cobra.Command{
		Use:   "sec-counter-web",
		Short: "Count Seconds since start",
		Long: `Count the seconds since the application got started in a self refreshing webpage.`,
		Run: runSecCounterCmd,
		Args:  cobra.ExactArgs(0),
	}

        secCounterCmd.Version = Version

	secCounterCmd.Flags().StringVarP(&dir, "dir", "d", dir, "directory to read files from")

	if err := secCounterCmd.Execute(); err != nil {
                os.Exit(1)
        }
}

func runSecCounterCmd(cmd *cobra.Command, args []string) {

        logerr := log.New(os.Stderr, "", log.LstdFlags)
        logger := log.New(os.Stdout, "", log.LstdFlags)

        done := make(chan bool)
        quit := make(chan os.Signal, 1)
        // interrupt signal sent from terminal
        signal.Notify(quit, os.Interrupt)
        // sigterm signal sent from kubernetes
        signal.Notify(quit, syscall.SIGTERM)
	// usage errs <- err
        errs := make(chan error)
	ticker := time.NewTicker(1 * time.Second)
	StartTime = time.Now()

	// webserver part
	if len(dir) > 0 {
                dir = dir + "/"
        }

	indexTemplate = template.Must(template.ParseFiles(dir + "index.template"))
	http.HandleFunc("/", indexHandler)
        server := &http.Server{Addr: ":8080"}
        server.SetKeepAlivesEnabled(false)

        go func() {
                <-quit
                logger.Println("Application is stopping...")
		ticker.Stop()
                close(done)
        }()

	go func() {
		for {
			select {
			case <-done:
				return
			case <-ticker.C:
				Seconds++
				//fmt.Printf("Start: %v, Seconds: %d\n",
				//	startTime.Format("2006-01-02 15:04:05"),
				//	Seconds)
			}
		}
	}()

	go func() {
		if err := server.ListenAndServe(); err != nil {
			errs <- err
		}
		close(done)
	}()


        select {
        case err := <-errs:
                logerr.Fatalf("Aborting... (%s)\n", err)
        case <-done:
                logger.Println("Counter stopped")
        }
}
type TemplateArgs struct {
	Message   string
        Hostname  string
        CurrTime  string
	StartTime string
	Uptime    string
}

func indexHandler(w http.ResponseWriter, r *http.Request) {
        hostname, err := os.Hostname()
        if err != nil {
                http.Error(w, "Can't get hostname", 500)
        }
        err = indexTemplate.Execute(w, TemplateArgs{
		Message:           "Alive since " + secondsToHuman(Seconds),
                Hostname:          hostname,
                CurrTime:          time.Now().Format("15:04:05"),
		StartTime:         StartTime.Format("15:04:05"),
		Uptime:            getUptime(),
        })
        if err != nil {
		log.Fatalf("template.Execute: %s\n", err)
                http.Error(w, "Can't execute template", 500)
        }
}

func getUptime() string {
        cmd := exec.Command("/bin/sh", "-c", "uptime | sed -e 's|.*up||g' -e 's|,.*||g'")
        var out bytes.Buffer
        cmd.Stdout = &out
        err := cmd.Run()
        if err == nil {
                return strings.TrimSpace(out.String())
        }
        return ""
}

func plural(count int, singular string) (result string) {
         if (count == 1) || (count == 0) {
                 result = strconv.Itoa(count) + " " + singular + " "
         } else {
                 result = strconv.Itoa(count) + " " + singular + "s "
         }
         return
 }

func secondsToHuman(input int) (result string) {
         years := math.Floor(float64(input) / 60 / 60 / 24 / 7 / 30 / 12)
         seconds := input % (60 * 60 * 24 * 7 * 30 * 12)
         months := math.Floor(float64(seconds) / 60 / 60 / 24 / 7 / 30)
         seconds = input % (60 * 60 * 24 * 7 * 30)
         weeks := math.Floor(float64(seconds) / 60 / 60 / 24 / 7)
         seconds = input % (60 * 60 * 24 * 7)
         days := math.Floor(float64(seconds) / 60 / 60 / 24)
         seconds = input % (60 * 60 * 24)
         hours := math.Floor(float64(seconds) / 60 / 60)
         seconds = input % (60 * 60)
         minutes := math.Floor(float64(seconds) / 60)
         seconds = input % 60

         if years > 0 {
                 result = plural(int(years), "year") + plural(int(months), "month") + plural(int(weeks), "week") + plural(int(days), "day") + plural(int(hours), "hour") + plural(int(minutes), "minute") + plural(int(seconds), "second")
         } else if months > 0 {
                 result = plural(int(months), "month") + plural(int(weeks), "week") + plural(int(days), "day") + plural(int(hours), "hour") + plural(int(minutes), "minute") + plural(int(seconds), "second")
         } else if weeks > 0 {
                 result = plural(int(weeks), "week") + plural(int(days), "day") + plural(int(hours), "hour") + plural(int(minutes), "minute") + plural(int(seconds), "second")
         } else if days > 0 {
                 result = plural(int(days), "day") + plural(int(hours), "hour") + plural(int(minutes), "minute") + plural(int(seconds), "second")
         } else if hours > 0 {
                 result = plural(int(hours), "hour") + plural(int(minutes), "minute") + plural(int(seconds), "second")
         } else if minutes > 0 {
                 result = plural(int(minutes), "minute") + plural(int(seconds), "second")
         } else {
                 result = plural(int(seconds), "second")
         }

         return
 }
