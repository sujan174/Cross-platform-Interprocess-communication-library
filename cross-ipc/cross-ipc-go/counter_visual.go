package main

import (
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"os/signal"
	"runtime"
	"strings"
	"syscall"
	"time"

	"github.com/hemanth2004/cross-ipc-go/cross_ipc"
)

// QueueData represents the JSON structure in shared memory
type QueueData struct {
	Counters   [][]int `json:"counters"`
	MaxCounter int     `json:"max_counter"`
}


const (
	Reset          = "\033[0m"
	Bold           = "\033[1m"
	Black          = "\033[30m"
	Red            = "\033[91m"
	Green          = "\033[92m"
	Yellow         = "\033[93m"
	Blue           = "\033[94m"
	Magenta        = "\033[95m"
	Cyan           = "\033[96m"
	White          = "\033[97m"
	BgBlack        = "\033[40m"
	BgRed          = "\033[41m"
	BgGreen        = "\033[42m"
	BgYellow       = "\033[43m"
	BgBlue         = "\033[44m"
	BgMagenta      = "\033[45m"
	BgCyan         = "\033[46m"
	BgWhite        = "\033[47m"
	BgBrightRed    = "\033[101m"
	BgBrightGreen  = "\033[102m"
	BgBrightYellow = "\033[103m"
	BgBrightBlue   = "\033[104m"
)

// Constants
const (
	PollInterval = 200 * time.Millisecond
	QueueSymbol  = "  " // Two spaces for wider blocks
)


func ClearScreen() {
	var cmd *exec.Cmd
	if runtime.GOOS == "windows" {
		cmd = exec.Command("cmd", "/c", "cls")
	} else {
		cmd = exec.Command("clear")
	}
	cmd.Stdout = os.Stdout
	cmd.Run()
}


func GetColorForValue(value int) string {
	// Define color ranges
	colors := []string{
		BgBlue,         
		BgCyan,         
		BgGreen,        
		BgBrightGreen,  // 96-127
		BgYellow,       
		BgBrightYellow, // 160-191
		BgRed,          // 192-223
		BgBrightRed,    // 224-255
	}

	
	if value < 0 {
		value = 0
	} else if value > 255 {
		value = 255
	}

	
	colorIndex := value / 32 
	return colors[colorIndex]
}


func DrawQueue(queueIdx int, queue []int, maxItems int) {
	
	fmt.Printf("")

	// Draw queue items with colors based on their values
	if len(queue) > 0 {
		fmt.Printf("%d  [", queueIdx+1)
		for _, value := range queue {
			bgColor := GetColorForValue(value)
			
			fmt.Printf("%s%s%s", bgColor, QueueSymbol, Reset)
		}

		
		emptyCount := maxItems - len(queue)
		if emptyCount > 0 {
			for i := 0; i < emptyCount; i++ {
				fmt.Print("··") 
			}
		}

		fmt.Printf("] %d/%d\n", len(queue), maxItems)
	} else {
		emptyStr := ""
		for i := 0; i < maxItems; i++ {
			emptyStr += "··" // Two dots for each empty slot
		}
		fmt.Printf("  [%s] (empty)\n", emptyStr)
	}

	fmt.Println() // Empty line for spacing
}

// HandleWindowsError handles Windows-specific error messages
func HandleWindowsError(err error) error {
	if err == nil {
		return nil
	}

	errMsg := err.Error()

	
	if strings.Contains(errMsg, "The operation completed successfully") ||
		strings.Contains(errMsg, "Cannot create a file when that file already exists") {
		return nil
	}

	return err
}

func main() {
	// Initialize shared memory
	shm, err := cross_ipc.NewSharedMemory("CounterSync", 4096, true)
	if err != nil {
		fmt.Printf("Failed to create shared memory: %v\n", err)
		return
	}

	success, err := shm.Setup()
	err = HandleWindowsError(err) 

	if err != nil {
		fmt.Printf("Failed to set up shared memory: %v\n", err)
		return
	}

	if !success && err == nil {
		fmt.Println("Warning: Setup returned false but no error was reported. Continuing anyway...")
	}

	fmt.Println("Connected to shared memory.")
	fmt.Println("Press Ctrl+C to exit.")
	time.Sleep(1 * time.Second)

	// Set up signal handling for graceful shutdown
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	// Create a ticker for periodic updates
	ticker := time.NewTicker(PollInterval)
	defer ticker.Stop()

	// Main loop
	running := true
	for running {
		select {
		case <-ticker.C:
			ClearScreen()

			
			dataStr, err := shm.Read()
			if err != nil {
				fmt.Printf("%sError reading shared memory: %v%s\n", Red, err, Reset)
				time.Sleep(PollInterval)
				continue
			}

			if dataStr == "" {
				fmt.Println("No data in shared memory yet. Make sure the producer is running.")
				time.Sleep(PollInterval)
				continue
			}

			// Parse JSON data
			var data QueueData
			if err := json.Unmarshal([]byte(dataStr), &data); err != nil {
				fmt.Printf("%sError parsing JSON: %v%s\n", Red, err, Reset)
				fmt.Printf("Raw data: %s\n", dataStr)
				time.Sleep(PollInterval)
				continue
			}

			
			if data.Counters == nil {
				fmt.Println("Invalid data structure, waiting for producer...")
				time.Sleep(PollInterval)
				continue
			}

			// Draw title
			fmt.Printf("\n%s%s===== Shared Memory Queue Visualizer =====%s\n\n", Cyan, Bold, Reset)

			// Draw color legend
			fmt.Printf("%sColor Legend (value ranges):%s\n", Bold, Reset)
			fmt.Printf("  %s%s%s 0-31   %s%s%s 32-63   %s%s%s 64-95   %s%s%s 96-127\n",
				BgBlue, QueueSymbol, Reset,
				BgCyan, QueueSymbol, Reset,
				BgGreen, QueueSymbol, Reset,
				BgBrightGreen, QueueSymbol, Reset)
			fmt.Printf("  %s%s%s 128-159   %s%s%s 160-191   %s%s%s 192-223   %s%s%s 224-255\n",
				BgYellow, QueueSymbol, Reset,
				BgBrightYellow, QueueSymbol, Reset,
				BgRed, QueueSymbol, Reset,
				BgBrightRed, QueueSymbol, Reset)
			fmt.Println()

			// Get max items per queue
			maxItems := data.MaxCounter
			if maxItems <= 0 {
				maxItems = 20
			}

			
			for i, queue := range data.Counters {
				DrawQueue(i, queue, maxItems)
			}

			
			fmt.Printf("\n%sPress Ctrl+C to quit%s\n", Cyan, Reset)

			// // Check if the shared memory is locked
			
			
			
			
			// 	fmt.Printf("%sShared memory is not locked%s\n", Green, Reset)
			

		case <-sigChan:
			fmt.Println("\nExiting...")
			running = false
		}
	}

	
	if err := shm.Close(); err != nil {
		err = HandleWindowsError(err)
		if err != nil {
			fmt.Printf("Error closing shared memory: %v\n", err)
		}
	}
	fmt.Println("Shared memory connection closed.")
}
