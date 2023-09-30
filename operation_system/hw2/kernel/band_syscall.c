#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/errno.h> 
#include <linux/signal.h>
#include <asm/current.h>
#include <linux/band.h>
//#include <errno.h>


#define SINGING 0
#define GUITAR 1
#define BASS 2
#define DRUMS 3

// Helper function to find a band by PID return a struct!!!
struct band* find_band_by_member(pid_t member) {

	struct task_struct* target = find_task_by_pid(member);
	if (target->band_music == NULL || target == NULL)
		return NULL;


	return target->band_music;


}

// Helper function to find the band of the current process
static struct band* find_band_of_current_process(void) {
	return find_band_by_member(current->pid);
}


// Helper function to check if an instrument is already played in the band
static int is_instrument_allocated(struct band* band_music, int instrument) {
	switch (instrument) {
	case SINGING:
		return band_music->singer >= 0;
	case GUITAR:
		return band_music->guitarist >= 0;
	case BASS:
		return band_music->bass >= 0;
	case DRUMS:
		return band_music->drummer >= 0;
	default:
		return  0;
	}
}

// Helper function to allocate an instrument in the band
static int allocate_instrument(struct band* band_music, int instrument, int pid) {
	switch (instrument) {
	case SINGING:
		band_music->singer = pid;
		break;
	case GUITAR:
		band_music->guitarist = pid;
		break;
	case BASS:
		band_music->bass = pid;
		break;
	case DRUMS:
		band_music->drummer = pid;
		break;
	default:
		return  -1;
	}
	return 0;
}

void update_the_band_when_exit(struct band* current_band)
{

	if (current_band != NULL)
	{
		int i;
		for (i = 0; i < 4; i++) {
			switch (i) {
			case SINGING:
				if (current_band->singer == current->pid) {
					current_band->singer = -1;
				}
				break;
			case GUITAR:
				if (current_band->guitarist == current->pid) {

					current_band->guitarist = -1;
				}
				break;
			case BASS:
				if (current_band->bass == current->pid) {
					current_band->bass = -1;
				}
				break;
			case DRUMS:
				if (current_band->drummer == current->pid) {
					current_band->drummer = -1;
				}

				break;
			default:
				return;
			}

		}

		//if needed delete the memory 
		if (current_band->singer < 0 && current_band->guitarist < 0
			&& current_band->bass < 0 && current_band->drummer < 0)
		{
			kfree(current_band);
			current_band = NULL;
		}

	}

}

// Define the system call implementations
int sys_band_create(int instrument) {


	struct task_struct* p = current;
	// Check instrument validity

	if (instrument < 0 || instrument > 3) {

		return  -EINVAL;
	}

	// Allocate memory for the new band
	struct band* new_band = (current->band_music) = kmalloc(sizeof(struct band), GFP_KERNEL);
	if (!new_band) {
		return -ENOMEM;
	}

	// Initialize band members
	new_band->singer = -1;
	new_band->guitarist = -1;
	new_band->bass = -1;
	new_band->drummer = -1;

	// Initialize note fields
	new_band->singer_note = -1;
	new_band->guitarist_note = -1;
	new_band->bass_note = -1;
	new_band->drummer_note = -1;

	int result = allocate_instrument(new_band, instrument, p->pid);
	if (result != 0) {
		kfree(new_band);
		return -1;
	}

	return 0;
}


int sys_band_join(pid_t member, int instrument) {

	pid_t modfied_pid;
	struct task_struct* p_current = current;
	if (member == 0) {
		modfied_pid = p_current->pid;

	}
	struct task_struct* p_target;
	if (member != 0) {
		p_target = find_task_by_pid(member);

	}
	else
	{
		p_target = p_current;
	}

	struct band* current_band = (p_current->band_music);

	// Check instrument validity
	if (instrument < 0 || instrument > 3) {
		return  -EINVAL;
	}

	if (p_target == NULL) {

		return -ESRCH;
	}
	struct band* target_band = (p_target->band_music);

	if (member == 0 && (current_band != NULL) && (current_band->singer != current->pid) && (current_band->guitarist != current->pid)
		&& (current_band->drummer != current->pid) && (current_band->bass != current->pid)) {
		return -EINVAL; // Member is not in a band
	}
	if (target_band == NULL)
	{
		return -EINVAL; // Member is not in a band

	}

	if (is_instrument_allocated(target_band, instrument)) {
		return -ENOSPC; // Instrument already allocated in the band
	}

	int result = allocate_instrument(target_band, instrument, p_current->pid);
	if (result != 0) { //failture
		return -1;
	}
	if (member != 0) {
		update_the_band_when_exit(current_band);
	}

	p_current->band_music = target_band;
	schedule();
	return 0;
}




/////////////////////////////////////////////////////////////////

int sys_band_play(int instrument, unsigned char note) {
	struct band* current_band = find_band_of_current_process();


	// Check instrument validity
	if (instrument < 0 || instrument > 3) {
		return -EINVAL;
	}


	if (!current_band) {
		return -ENOENT; // Process is not in any band

	}


	switch (instrument) {
	case SINGING:

		if (current_band->singer != current->pid)
		{
			return -EACCES;
		}
		if (current_band->singer_note == -1)
		{
			current_band->singer_note = note;

		}
		else
		{
			return -EBUSY;

		}
		break;

	case GUITAR:
		if (current_band->guitarist != current->pid)
		{
			return -EACCES;
		}
		if (current_band->guitarist_note == -1)
		{

			current_band->guitarist_note = note;
		}
		else
		{
			return -EBUSY;

		}
		break;
	case BASS:
		if (current_band->bass != current->pid)
		{
			return -EACCES;
		}
		if (current_band->bass_note == -1)
		{
			current_band->bass_note = note;
		}
		else
		{
			return -EBUSY;

		}

		break;
	case DRUMS:
		if (current_band->drummer != current->pid)
		{
			return -EACCES;
		}
		if (current_band->drummer_note == -1)
		{
			current_band->drummer_note = note;
		}
		else
		{
			return -EBUSY;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}



int sys_band_listen(pid_t member, unsigned char* chord) {

	struct task_struct* target = find_task_by_pid(member);

	if (chord == NULL) {
		return -EFAULT;
	}
	if (target == NULL) {
		return -ESRCH;

	}
	struct band* requested_band = target->band_music;

	if (requested_band == NULL) {
		return -EINVAL; // Process is not in any band

	}

	if (requested_band->singer_note == -1 || requested_band->bass_note == -1
		|| requested_band->drummer_note == -1 || requested_band->guitarist_note == -1)
	{
		return -EAGAIN; // Not all instruments played yet
	}


	chord[SINGING] = requested_band->singer_note;
	requested_band->singer_note = -1;

	chord[GUITAR] = requested_band->guitarist_note;
	requested_band->guitarist_note = -1;

	chord[BASS] = requested_band->bass_note;
	requested_band->bass_note = -1;

	chord[DRUMS] = requested_band->drummer_note;
	requested_band->drummer_note = -1;

	return 0;
}






