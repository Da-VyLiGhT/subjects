#include "ListNode.h"

ListNode* SolveB(int n) {
	/* your code starts here */
	//construct a list accrording to input
	int value;
	cin >> value;
	ListNode h(0);
	ListNode* head = &h;
	head->val = value;
	ListNode t(0);
	ListNode* temp = &t;
	head->next = temp;
	ListNode Eh(0);
	ListNode* Ehead = &Eh;
	ListNode Et(0);
	ListNode* Etemp = &Et;
	ListNode Oh(0);
	ListNode* Ohead = &Oh;
	ListNode Ot(0);
	ListNode* Otemp = &Ot;
	for (int i = 1; i < n; i++) {	
		if (i <= n - 2) {
			cin >> value;
			temp->val = value;
			ListNode* p = new ListNode(0);
			temp->next = p;
			temp = p;
		}
		else {
			cin >> value;
			temp->val = value;
			temp->next = NULL;
		}
	}
	//rearrange the list
	if (n % 2 == 0) {

		Ehead->val = (head->next)->val;
		Ehead->next = Etemp;
		temp = head->next;
		for (int j = 2; j <= (n / 2); j++) {
			Etemp->val = temp->next->next->val;
			Etemp->next = temp->next->next;
			temp = temp->next->next;
		}

		Ohead->val = head->val;
		Ohead->next = Otemp;
		temp = head;
			for (int j = 2; j <= (n / 2); j++) {
				if (j == n / 2) {
					Otemp->val = temp->next->next->val;
					Otemp->next = Ehead;
				}
				else {
					Otemp->val = temp->next->next->val;
					Otemp->next = temp->next->next;
					temp = temp->next->next;
				}
			}

	}
	else {

		Ehead->val = (head->next)->val;
		Ehead->next = Etemp;
		temp = head->next;
		for (int j = 2; j <= (n - 1 / 2); j++) {
			Etemp->val = temp->next->next->val;
			Etemp->next = temp->next->next;
			temp = temp->next->next;
		}

		Ohead->val = head->val;
		Ohead->next = Otemp;
		temp = head;
			for (int j = 2; j <= (n + 1 / 2); j++) {
				if (j == n / 2) {
					Otemp->val = temp->next->next->val;
					Otemp->next = Ehead;
				}
				else {
					Otemp->val = temp->next->next->val;
					Otemp->next = temp->next->next;
					temp = temp->next->next;
				}
			}
	}

	return Ohead;

}

int main() {
	int n;
	cin >> n;
	// get the head node of rearranged list
	ListNode* ls = SolveB(n);
	for (int i = 0; i < n; i++) {
		cout << ls->val << " ";
		ls = ls->next;
	}
	return 0;
}