/** 
 * @file csma.cpp
 * @brief A toy simulation of the Carrier Sense Multiple Access (CSMA) protocol.
 *
 * This file contains the function implementations and main() function
 * of the CSMA simulation. 
 *
 * @author Vicky Chen (chen-vv)
 * @author Eric Omielan (eomielan)
 * @bug No known bugs.
 */

/* -- Includes -- */

/* Standard library includes. */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <iomanip>

/* Custom includes */
#include "include/csma.h"

void assign_values(std::ifstream& input_file) {
    std::string line;

    while (std::getline(input_file, line)) {
        std::stringstream ss(line);
        char parameter;
        ss >> parameter;

        switch (parameter) {
            case 'N': {
                int N;
                ss >> N;
                nodes.resize(N);
                break;
            }

            case 'L':
                ss >> packet_length;
                break;

            case 'M':
                ss >> max_retransmission_attempt;
                break;

            case 'R': {
                int r_value;
                while (ss >> r_value) {
                    R.push_back(r_value);
                }
                break;
            }

            case 'T':
                ss >> total_simulation_time;
                break;

            default:
                std::cerr << "Unknown parameter: " << parameter << std::endl;
        }
    }
}

int generate_backoff(int node_id, int ticks, int R) {
    int backoff = (node_id + ticks) % R;
    return backoff;
}

void set_channel_occupied(bool is_occupied) {
    channel_occupied = is_occupied;
}

std::vector<int> get_ready_node_ids() {
    std::vector<int> ready_nodes;

    for (const auto& node : nodes) {
        if (node.backoff == READY_TO_TRANSMIT) {
            ready_nodes.push_back(node.id);
        }
    }

    return ready_nodes;
}

void initialize_nodes() {
    int curr_id = 0;
    
    for (auto& node : nodes) {
        node.id = curr_id++;
        node.collision_count = 0;
        node.R = R[0];
        node.backoff = generate_backoff(node.id, 0, node.R);
    }
}

void transmit_packet(int active_node_id, int ticks) {
    std::cout << "Channel is occupied by node " << active_node_id << std::endl;

    Node& active_node = nodes[active_node_id];
    active_node.packet_ticks_remaining--;

    if (active_node.packet_ticks_remaining == TRANSMIT_COMPLETE) {
        active_node.R = R[0];
        active_node.collision_count = 0;
        active_node.backoff = generate_backoff(active_node.id, ticks + 1, active_node.R);
        set_channel_occupied(false);

        std::cout << "Node " << active_node_id << " finished transmitting. new backoff " << nodes[active_node_id].backoff  << std::endl;
    }

    num_successful_transmission_ticks++;
}

/** 
 * @brief The CSMA simulation entrypoint.
 *
 * This function is the entry point of the CSMA simulation program. 
 * It processes command line arguments to determine the input file,
 * simulates the CSMA protocol, then writes the link utilization rate
 * to an output file. The output file is named "output.txt" by default,
 * though the user may specify an alternative name via the command line.
 * 
 * In the simulation, each tick of the simulated clock is represented
 * by one iteration of the main loop. In each tick, the program checks
 * the status of the channel and the nodes. 
 * 
 * If the channel is idle, the
 * program checks if any nodes are ready to transmit. If there is only
 * one node ready to transmit, the node begins transmitting. If there
 * are multiple nodes ready to transmit, a collision is detected and
 * the nodes must back off and retransmit.
 * 
 * If the channel is occupied, the program checks if the node currently
 * transmitting has finished transmitting. If the node has finished
 * transmitting, the channel is marked as idle and the node backs off and
 * waits for the next opportunity to retransmit a new packet. 
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return Returns EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main(int argc, char* argv[]) {
    // Check for the correct number of arguments
    if (argc != 2 && argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <inputfilename> [outputfilename]" << std::endl;
        return EXIT_FAILURE;
    }

    // Open the input file
    std::ifstream input_file(argv[1]);

    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    assign_values(input_file);

    input_file.close();

    // For each node, initialize its status and other properties
    initialize_nodes();

    channel_occupied = false;
    num_successful_transmission_ticks = 0;

    for (int ticks = 0; ticks < total_simulation_time; ticks++) {
        std::cout << "Tick: " << ticks << std::endl;
        for (auto& node : nodes) {
            std::cout << "Node " << node.id << " backoff: " << node.backoff << std::endl;
        }

        if (channel_occupied) {
            transmit_packet(active_node_id, ticks);
        } else {
            std::vector<int> ready_nodes = get_ready_node_ids();

            if (ready_nodes.empty()) {
                // No nodes are ready to transmit
                std::cout << "Channel is idle.\n" << std::endl;

                for (auto& node : nodes) {
                    node.backoff--;
                }
            } else if (ready_nodes.size() == 1) {
                // Only one node is ready to transmit
                set_channel_occupied(true);

                active_node_id = ready_nodes[0];
                nodes[active_node_id].packet_ticks_remaining = packet_length;

                transmit_packet(active_node_id, ticks);
            } else {
                // Multiple nodes are ready to transmit, so a collision occurs
                std::cout << "Collision detected b/w:" << std::endl;

                for (int node_id : ready_nodes) {
                    Node& node = nodes[node_id];

                    std::cout << "Node " << node.id << std::endl;

                    node.collision_count++;

                    if (node.collision_count > max_retransmission_attempt) {
                        // Drop packet and reset node
                        node.R = R[0];
                        node.collision_count = 0;
                        node.backoff = generate_backoff(node.id, ticks + 1, node.R);
                        continue;
                    }

                    node.R = R[node.collision_count];
                    node.backoff = generate_backoff(node.id, ticks + 1, node.R);
                }
            }
        }
    }

    // Write the link utilization rate to the output file
    std::ofstream output_file;
    if (argc == 3) {
        output_file.open(argv[2]);
    } else {
        output_file.open("output.txt");
    }

    if (!output_file.is_open()) {
        std::cerr << "Error: Unable to open file output.txt" << std::endl;
        return EXIT_FAILURE;
    }

    output_file << std::fixed << std::setprecision(2);
    output_file << static_cast<double>(num_successful_transmission_ticks) / total_simulation_time << std::endl;

    std::cout << "Slots with succcessful transmissions: " << num_successful_transmission_ticks << ", T = " << total_simulation_time << std::endl;

    output_file.close();

    return EXIT_SUCCESS;
}
