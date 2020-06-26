// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.helper;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Map.Entry;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.DefaultTableModel;

import org.opt4j.core.config.Property;

@SuppressWarnings("serial")
public class EnvironmentPanel extends JScrollPane {

    public EnvironmentPanel(Property property) {
        Environment value = (Environment) property.getValue();

        final DefaultTableModel model = new DefaultTableModel(
                new Object[]{"Environment variable", "value"}, 0);
        for (Entry<String, String> e : value.entrySet()) {
            model.addRow(new Object[]{e.getKey(), e.getValue()});
        }
        model.addRow(new Object[]{"foo", "bar"});
        final JTable table = new JTable(model);
        this.setViewportView(table);

        final JMenuItem menuItemAdd = new JMenuItem("Add New Row");
        final JMenuItem menuItemRemove = new JMenuItem("Remove Current Row");
        final JMenuItem menuItemRemoveAll = new JMenuItem("Remove All Rows");
        {
            ActionListener menuActionListener = new ActionListener() {

                @Override
                public void actionPerformed(ActionEvent e) {
                    // TODO Auto-generated method stub

                }


            };
            menuItemAdd.addActionListener(menuActionListener);
            menuItemRemove.addActionListener(menuActionListener);
            menuItemRemoveAll.addActionListener(menuActionListener);
        }

        {
            JPopupMenu popupMenu = new JPopupMenu();
            popupMenu.add(menuItemAdd);
            popupMenu.add(menuItemRemove);
            popupMenu.add(menuItemRemoveAll);
            // Set the popup menu for the table
            table.setComponentPopupMenu(popupMenu);
        }

        {
            JPopupMenu popupMenu = new JPopupMenu();
            popupMenu.add(menuItemAdd);
            popupMenu.add(menuItemRemoveAll);
            // Set the popup menu for the table
            this.setComponentPopupMenu(popupMenu);
        }

//      System.err.println(table.getMinimumSize());
//      System.err.println(table.getPreferredSize());
//      System.err.println(scrollPane.getMinimumSize());
//      System.err.println(scrollPane.getPreferredSize());
        this.setMinimumSize(new Dimension(-1, 22*4));
        this.setPreferredSize(this.getMinimumSize());

    }
}
